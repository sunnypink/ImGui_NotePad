/*
 * ImGui_FileManager.cpp
 *
 *  Created on: Dec 28, 2023
 *      Author: sunny
 */
#include "ImGui_FileManager.hpp"
#include <chrono>
#include <ctime>
// Enforce cdecl calling convention for functions called by the standard library, in case compilation settings changed the default to e.g. __vectorcall
#ifndef IMGUI_CDECL
#ifdef _MSC_VER
#define IMGUI_CDECL __cdecl
#else
#define IMGUI_CDECL
#endif
#endif

/* Order of write program
 * 1. draw window.
 * 2. read directory and files from linux and show the window.
 * 3. select the directory and files from user.
 * 4. build a design. (this the las thing)*
 * */


/*
 *         IMGUI_DEMO_MARKER("Widgets/Selectables/Single Selection");
        if (ImGui::TreeNode("Selection State: Single Selection"))
        {
            static int selected = -1;
            for (int n = 0; n < 5; n++)
            {
                char buf[32];
                sprintf(buf, "Object %d", n);
                if (ImGui::Selectable(buf, selected == n))
                    selected = n;
            }
            ImGui::TreePop();
        }
 */

enum TableColumnID
{
    TableColumnID_Name,
    TableColumnID_Type,
    TableColumnID_Size,
    TableColumnID_Time
};

struct TableItem
{
	const char *name;
	const char *type;
	int size;
	std::filesystem::file_time_type time;


	// We have a problem which is affecting _only this demo_ and should not affect your code:
    // As we don't rely on std:: or other third-party library to compile dear imgui, we only have reliable access to qsort(),
    // however qsort doesn't allow passing user data to comparing function.
    // As a workaround, we are storing the sort specs in a static/global for the comparing function to access.
    // In your own use case you would probably pass the sort specs to your sorting/comparing functions directly and not use a global.
    // We could technically call ImGui::TableGetSortSpecs() in CompareWithSortSpecs(), but considering that this function is called
    // very often by the sorting algorithm it would be a little wasteful.
    static const ImGuiTableSortSpecs* s_current_sort_specs;

    static void SortWithSortSpecs(ImGuiTableSortSpecs* sort_specs, TableItem* items, int items_count)
    {
        s_current_sort_specs = sort_specs; // Store in variable accessible by the sort function.
        if (items_count > 1)
            qsort(items, (size_t)items_count, sizeof(items[0]), TableItem::CompareWithSortSpecs);
        s_current_sort_specs = NULL;
    }

    // Compare function to be used by qsort()
    static int IMGUI_CDECL CompareWithSortSpecs(const void* lhs, const void* rhs)
    {
        const TableItem* a = (const TableItem*)lhs;
        const TableItem* b = (const TableItem*)rhs;
        for (int n = 0; n < s_current_sort_specs->SpecsCount; n++)
        {
            // Here we identify columns using the ColumnUserID value that we ourselves passed to TableSetupColumn()
            // We could also choose to identify columns based on their index (sort_spec->ColumnIndex), which is simpler!
            const ImGuiTableColumnSortSpecs* sort_spec = &s_current_sort_specs->Specs[n];
            int delta = 0;
            switch (sort_spec->ColumnUserID)
            {
				case TableColumnID_Name:             delta = (strcmp(a->name, b->name));                break;
				case TableColumnID_Type:           	 delta = (strcmp(a->type, b->type));     break;
				case TableColumnID_Size:   		 	delta = (a->size - b->size);    break;
				default: IM_ASSERT(0); break;
            }
            if (delta > 0)
                return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? +1 : -1;
            if (delta < 0)
                return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? -1 : +1;
        }

        // qsort() is instable so always return a way to differenciate items.
        // Your own compare function may want to avoid fallback on implicit sort specs e.g. a Name compare if it wasn't already part of the sort specs.
        return (a->size - b->size);
    }
};
	const ImGuiTableSortSpecs* TableItem::s_current_sort_specs = NULL;


/*Default Constructor*/
ImGui_FileExplore::ImGui_FileExplore()
{
	this->m_gui_dirContentInfo = fileManager.GetDirectoryInfo();

}

extern void imgui_window_marker(char *section);
void ImGui_FileExplore::ImGui_FileWindowDraw(const char* title, bool* p_open)
{
    ImGui::SetNextWindowSize(ImVec2(800, 500), ImGuiCond_None);

    if (!ImGui::Begin(title, p_open))
    {
        ImGui::End();
        return;
    }

    // As a specific feature guaranteed by the library, after calling Begin() the last Item represent the title bar.
    // So e.g. IsItemHovered() will return true when hovering the title bar.
    // Here we create a context menu only available from the title bar.
    if (ImGui::BeginPopupContextItem())
    {
        if (ImGui::MenuItem("Close window"))
            *p_open = false;
        ImGui::EndPopup();
    }


    ImGui_ReadDirectoryList(this->m_gui_dirContentInfo);
	ImGui_SelectDirectoryPath(this->m_gui_dirContentInfo);

    ImGui::End();
}



const char* ImGui_FileExplore::to_string(const std::filesystem::file_time_type& ftime)
{

    // Convert the file_time_type to a more easily printable representation
    std::chrono::system_clock::time_point timePoint = std::chrono::time_point_cast<std::chrono::system_clock::duration>(ftime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());

    // Convert the time_point to a std::time_t value
    std::time_t time = std::chrono::system_clock::to_time_t(timePoint);

    // Format and print the last modified time
    std::strftime(this->time_buffer, sizeof(this->time_buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&time));
    //std::cout << "Last modified time: " << this->time_buffer << std::endl;
    return this->time_buffer;

}

//Top Area, Select directory or file used by path
void ImGui_FileExplore::ImGui_SelectDirectoryPath(FileManager::directory_content_info &dirContentInfo)
{
    //IMGUI_DEMO_MARKER("Widgets/Selectables/Single Selection");
    //IMGUI_DEMO_MARKER("Widgets/List Boxes");
	char marker[] = "Widgets/Selectables";
	imgui_window_marker(marker);

//	std::cout<<"ImGui_SelectDirectoryPath"<<std::endl;

    static ImGuiTableFlags flags =
        ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable
        | ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti
        | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody
        | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY
        | ImGuiTableFlags_SizingStretchProp;
    static ImGuiTableColumnFlags columns_base_flags = ImGuiTableColumnFlags_None;

    enum ContentsType { CT_Text, CT_Button, CT_SmallButton, CT_FillButton, CT_Selectable, CT_SelectableSpanRow };
    static int contents_type = CT_SelectableSpanRow;
    static int items_count = this->m_gui_dirContentInfo.dirList.size();

    static int freeze_cols = 1;
    static int freeze_rows = 1;

    static ImVec2 outer_size_value = ImVec2(0.0f, TEXT_BASE_HEIGHT * 12);
    static float row_min_height = 0.0f; // Auto
    static float inner_width_with_scroll = 0.0f; // Auto-extend
    static bool outer_size_enabled = false;
    static bool show_headers = true;
    static bool show_wrapped_text = false;
    static ImGuiChildFlags child_flags =  ImGuiChildFlags_ResizeX | ImGuiChildFlags_ResizeY;

    static int item_current_idx = 0; // Here we store our selection data as an index.
	int dirPathList = m_gui_dirContentInfo.rootPathList.size();
    const char* combo_preview_value = m_gui_dirContentInfo.dirList[0].currentPath.c_str();  // Pass in the preview value visible before opening the combo (it could be anything)


    ImGui::Spacing();
    ImGui::BeginChild("Red", ImVec2(700, 100), child_flags, ImGuiWindowFlags_None);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(20, 7));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(100, 10)); // indenting of object
    ImGui::Indent();
    ImGui::Indent();
    ImGui::SameLine();
    if (ImGui::BeginCombo("DIR", combo_preview_value, ImGuiComboFlags_WidthFitPreview))
    {
        for (int n = 0; n < dirPathList; n++)
        {
            const bool is_selected = (item_current_idx == n);
            if (ImGui::Selectable(m_gui_dirContentInfo.rootPathList[n].c_str(), is_selected))
                item_current_idx = n;

            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    ImGui::PopStyleVar(2);
    //ImGui::PopStyleVar();
    ImGui::EndChild();
    ImGui::Spacing();

    // Update item list if we changed the number of items
    // Table Part
    const ImDrawList* parent_draw_list = ImGui::GetWindowDrawList();
    const int parent_draw_list_draw_cmd_count = parent_draw_list->CmdBuffer.Size;
    ImVec2 table_scroll_cur, table_scroll_max; // For debug display
    const ImDrawList* table_draw_list = NULL;  // "

    // Update direcotry and file  list
    static ImVector<TableItem> table_item;
    static ImVector<int> selection;
    static bool items_need_sort = false;
    if (table_item.Size != items_count)
    {
    	table_item.resize(items_count, TableItem());
        for (int n = 0; n < items_count; n++)
        {
            const int template_n = n % this->m_gui_dirContentInfo.dirList.size();
            TableItem& item = table_item[n];
            item.name = this->m_gui_dirContentInfo.dirList[template_n].name.c_str();
            item.size = this->m_gui_dirContentInfo.dirList[template_n].number_subitem;
            item.type = this->m_gui_dirContentInfo.dirList[template_n].extension.c_str();
            item.time = this->m_gui_dirContentInfo.dirList[template_n].last_modified_time;
        }
    }//end if


    // Submit table
    const float inner_width_to_use = (flags & ImGuiTableFlags_ScrollX) ? inner_width_with_scroll : 0.0f;
    if (ImGui::BeginTable("table_advanced", 4, flags, outer_size_enabled ? outer_size_value : ImVec2(0, 0), inner_width_to_use))
    {
        // Declare columns
        // We use the "user_id" parameter of TableSetupColumn() to specify a user id that will be stored in the sort specifications.
        // This is so our sort function can identify a column given our own identifier. We could also identify them based on their index!

        ImGui::TableSetupColumn("Name",         columns_base_flags | ImGuiTableColumnFlags_WidthFixed, 0.0f, TableColumnID_Name);
        ImGui::TableSetupColumn("Type",       columns_base_flags | ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed, 0.0f, TableColumnID_Type);
        ImGui::TableSetupColumn("Size",     columns_base_flags | ImGuiTableColumnFlags_PreferSortDescending, 0.0f, TableColumnID_Size);
        ImGui::TableSetupColumn("Time",       columns_base_flags |  ImGuiTableColumnFlags_NoSort);
        ImGui::TableSetupScrollFreeze(freeze_cols, freeze_rows);

        // Sort our data if sort specs have been changed!
        ImGuiTableSortSpecs* sort_specs = ImGui::TableGetSortSpecs();
        if (sort_specs && sort_specs->SpecsDirty)
            items_need_sort = true;
        if (sort_specs && items_need_sort && table_item.Size > 1)
        {
            TableItem::SortWithSortSpecs(sort_specs, table_item.Data, table_item.Size);
            sort_specs->SpecsDirty = false;
        }
        items_need_sort = false;

        // Take note of whether we are currently sorting based on the Quantity field,
        // we will use this to trigger sorting when we know the data of this column has been modified.
        const bool sorts_specs_using_quantity = (ImGui::TableGetColumnFlags(0) & ImGuiTableColumnFlags_IsSorted) != 0;

        // Show headers
        if (show_headers && (columns_base_flags & ImGuiTableColumnFlags_AngledHeader) != 0)
            ImGui::TableAngledHeadersRow();
        if (show_headers)
            ImGui::TableHeadersRow();

        // Show data
        // FIXME-TABLE FIXME-NAV: How we can get decent up/down even though we have the buttons here?
        ImGui::PushButtonRepeat(true);
#if 1
        // Demonstrate using clipper for large vertical lists
        ImGuiListClipper clipper;
        clipper.Begin(table_item.Size);
        while (clipper.Step())
        {
            for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++)
#else
        // Without clipper
        {
            for (int row_n = 0; row_n < items.Size; row_n++)
#endif
            {
                TableItem* item = &table_item[row_n];
                //if (!filter.PassFilter(item->Name))
                //    continue;

                const bool item_is_selected = selection.contains(item->name[0]);
                ImGui::PushID(item->name);
                ImGui::TableNextRow(ImGuiTableRowFlags_None, row_min_height);

                // For the demo purpose we can select among different type of items submitted in the first column
                ImGui::TableSetColumnIndex(0);
                char label[32];
                sprintf(label, "%s", item->name);
                if (contents_type == CT_Selectable || contents_type == CT_SelectableSpanRow)
                {
                    ImGuiSelectableFlags selectable_flags = (contents_type == CT_SelectableSpanRow) ? ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap : ImGuiSelectableFlags_None;
                    if (ImGui::Selectable(label, item_is_selected, selectable_flags, ImVec2(0, row_min_height)))
                    {
                        if (ImGui::GetIO().KeyCtrl)
                        {
                            if (item_is_selected)
                                selection.find_erase_unsorted(item->name[0]);
                            else
                                selection.push_back(item->name[0]);
                        }
                        else
                        {
                            selection.clear();
                            selection.push_back(item->name[0]);
                        }
                    }
                }

                // Here we demonstrate marking our data set as needing to be sorted again if we modified a quantity,
                // and we are currently sorting on the column showing the Quantity.
                // To avoid triggering a sort while holding the button, we only trigger it when the button has been released.
                // You will probably need a more advanced system in your code if you want to automatically sort when a specific entry changes.

					ImGui::TableSetColumnIndex(1); // description of type
					ImGui::Text("%s", item->type);

				if (ImGui::TableSetColumnIndex(2)) //size
					ImGui::Text("%d", item->size);

                /* not button
                if (ImGui::TableSetColumnIndex(2))
                {
                    if (ImGui::SmallButton("Chop")) { item->size += 1; }
                    if (sorts_specs_using_quantity && ImGui::IsItemDeactivated()) { items_need_sort = true; }
                    ImGui::SameLine();
                    if (ImGui::SmallButton("Eat")) { item->size -= 1; }
                    if (sorts_specs_using_quantity && ImGui::IsItemDeactivated()) { items_need_sort = true; }
                }*/
                if (ImGui::TableSetColumnIndex(3))//modified time
                    ImGui::Text(" %s ", to_string(item->time));

                ImGui::PopID();
            }
        }
        ImGui::PopButtonRepeat();

        // Store some info to display debug details below
        table_scroll_cur = ImVec2(ImGui::GetScrollX(), ImGui::GetScrollY());
        table_scroll_max = ImVec2(ImGui::GetScrollMaxX(), ImGui::GetScrollMaxY());
        table_draw_list = ImGui::GetWindowDrawList();
        ImGui::EndTable();
    }//end while


}//end function

/* Need to Porting It's nightmare
    ShowDemoWindowColumns(); // actual show table
*/
#if 0
    char marker2[]="Widgets/Selectables/Single Selection";
	imgui_window_marker(marker2);

	ImGui::Spacing();
    static ImGuiChildFlags child_flags =  ImGuiChildFlags_ResizeX | ImGuiChildFlags_ResizeY;
    static int item_current_idx = 0; // Here we store our selection data as an index.
	int size = dirContentInfo.dirList.size();
    const char* combo_preview_value = dirContentInfo.dirList[0].path.c_str();  // Pass in the preview value visible before opening the combo (it could be anything)
    ImGui::BeginChild("Red", ImVec2(700, 100), child_flags, ImGuiWindowFlags_None);
    ImGui::Indent();
    ImGui::Indent();
    ImGui::SameLine();
    if (ImGui::BeginCombo("DIR", combo_preview_value, ImGuiComboFlags_WidthFitPreview))
    {
        for (int n = 0; n < size; n++)
        {
            const bool is_selected = (item_current_idx == n);
            if (ImGui::Selectable(dirContentInfo.dirList[n].path.c_str(), is_selected))
                item_current_idx = n;

            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    ImGui::EndChild();
    ImGui::Spacing();
    ImGui::Spacing();



    /*Show the selected item */
    ImGui::BeginChild("Red", ImVec2(700, 100), child_flags, ImGuiWindowFlags_None);
    static ImGuiTableFlags sizing_policy_flags[4] = { ImGuiTableFlags_SizingFixedFit, ImGuiTableFlags_SizingFixedSame, ImGuiTableFlags_SizingStretchProp, ImGuiTableFlags_SizingStretchSame };
    for (int table_n = 0; table_n < 4; table_n++)
    {
        ImGui::PushID(table_n);
        ImGui::SetNextItemWidth(TEXT_BASE_WIDTH * 30);
        EditTableSizingFlags(&sizing_policy_flags[table_n]);

        // To make it easier to understand the different sizing policy,
        // For each policy: we display one table where the columns have equal contents width, and one where the columns have different contents width.
        if (ImGui::BeginTable("table1", 3, sizing_policy_flags[table_n] | flags1))
        {
            for (int row = 0; row < 3; row++)
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn(); ImGui::Text("Oh dear");
                ImGui::TableNextColumn(); ImGui::Text("Oh dear");
                ImGui::TableNextColumn(); ImGui::Text("Oh dear");
            }
            ImGui::EndTable();
        }
        if (ImGui::BeginTable("table2", 3, sizing_policy_flags[table_n] | flags1))
        {
            for (int row = 0; row < 3; row++)
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn(); ImGui::Text("AAAA");
                ImGui::TableNextColumn(); ImGui::Text("BBBBBBBB");
                ImGui::TableNextColumn(); ImGui::Text("CCCCCCCCCCCC");
            }
            ImGui::EndTable();
        }
        ImGui::PopID();
    }



    ImGui::EndChild();
#endif
#if 0
    // Submit selected items so we can query their status in the code following it.
    bool ret = false;
    static bool b = false;
    static float col4f[4] = { 1.0f, 0.5, 0.0f, 1.0f };
    static char str[16] = {};
    if (item_disabled)
        ImGui::BeginDisabled(true);
    if (item_type == 0) { ImGui::Text("ITEM: Text"); }                                              // Testing text items with no identifier/interaction
    if (item_type == 1) { ret = ImGui::Button("ITEM: Button"); }                                    // Testing button
    if (item_type == 2) { ImGui::PushButtonRepeat(true); ret = ImGui::Button("ITEM: Button"); ImGui::PopButtonRepeat(); } // Testing button (with repeater)
    if (item_type == 3) { ret = ImGui::Checkbox("ITEM: Checkbox", &b); }                            // Testing checkbox
    if (item_type == 4) { ret = ImGui::SliderFloat("ITEM: SliderFloat", &col4f[0], 0.0f, 1.0f); }   // Testing basic item
    if (item_type == 5) { ret = ImGui::InputText("ITEM: InputText", &str[0], IM_ARRAYSIZE(str)); }  // Testing input text (which handles tabbing)
    if (item_type == 6) { ret = ImGui::InputTextMultiline("ITEM: InputTextMultiline", &str[0], IM_ARRAYSIZE(str)); } // Testing input text (which uses a child window)
    if (item_type == 7) { ret = ImGui::InputFloat("ITEM: InputFloat", col4f, 1.0f); }               // Testing +/- buttons on scalar input
    if (item_type == 8) { ret = ImGui::InputFloat3("ITEM: InputFloat3", col4f); }                   // Testing multi-component items (IsItemXXX flags are reported merged)
    if (item_type == 9) { ret = ImGui::ColorEdit4("ITEM: ColorEdit4", col4f); }                     // Testing multi-component items (IsItemXXX flags are reported merged)
    if (item_type == 10){ ret = ImGui::Selectable("ITEM: Selectable"); }                            // Testing selectable item
    if (item_type == 11){ ret = ImGui::MenuItem("ITEM: MenuItem"); }                                // Testing menu item (they use ImGuiButtonFlags_PressedOnRelease button policy)
    if (item_type == 12){ ret = ImGui::TreeNode("ITEM: TreeNode"); if (ret) ImGui::TreePop(); }     // Testing tree node
    if (item_type == 13){ ret = ImGui::TreeNodeEx("ITEM: TreeNode w/ ImGuiTreeNodeFlags_OpenOnDoubleClick", ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_NoTreePushOnOpen); } // Testing tree node with ImGuiButtonFlags_PressedOnDoubleClick button policy.
    if (item_type == 14){ const char* items[] = { "Apple", "Banana", "Cherry", "Kiwi" }; static int current = 1; ret = ImGui::Combo("ITEM: Combo", &current, items, IM_ARRAYSIZE(items)); }
    if (item_type == 15){ const char* items[] = { "Apple", "Banana", "Cherry", "Kiwi" }; static int current = 1; ret = ImGui::ListBox("ITEM: ListBox", &current, items, IM_ARRAYSIZE(items), IM_ARRAYSIZE(items)); }
#endif


void ImGui_FileExplore::ImGui_ReadDirectoryList(FileManager::directory_content_info &m_dirContentInfo)
{

	/*1. read current directory used ${PWD}*/
	m_dirContentInfo = fileManager.GetDirectoryInfo();
	//ImGui_FileManageBuildList();

	//std::cout<<"current_direcotry_path : "<<dir_info.current_directory_path<<std::endl;
	/*2. show the list current directory list*/
	//ImGui::TextWrapped((char *)dir_info.current_directory_path.c_str());

}
#if 0
void ImGui_FileExplore::ImGui_FileManageBuildList()
{

	std::cout << "ImGui_FileManagePrint" << std::endl;

	for(int i=0; i < m_dirVectorListSize; i++)
	{
		std::cout<<"directory: " << this->m_gui_dirContentInfo.dirList.at(i).path << std::endl;
		//this->m_gui_path[i] = this->m_gui_dirContentInfo.dirList.at(i).path.c_str();
		//this->m_gui_name[i] = this->m_gui_dirContentInfo.dirList.at(i).name.c_str();
		strncpy(this->mptr_gui_path_list[i], this->m_gui_dirContentInfo.dirList.at(i).path.c_str(), this->m_gui_dirContentInfo.dirList.at(i).path.size());
		strncpy(this->mptr_gui_name_list[i], this->m_gui_dirContentInfo.dirList.at(i).name.c_str(), this->m_gui_dirContentInfo.dirList.at(i).name.size());
	}
	for(int i=0; i < m_fileVectorListSize; i++)
	{
		std::cout<<"file: " << this->m_gui_dirContentInfo.fileList.at(i).path << std::endl;
		//this->m_gui_path[i+dir_size] = this->m_gui_dirContentInfo.fileList.at(i).path.c_str();
		//this->m_gui_name[i+dir_size] = this->m_gui_dirContentInfo.fileList.at(i).name.c_str();
		strncpy(this->mptr_gui_path_list[i+m_dirVectorListSize], this->m_gui_dirContentInfo.fileList.at(i).path.c_str(), this->m_gui_dirContentInfo.fileList.at(i).path.size());
		strncpy(this->mptr_gui_name_list[i+m_dirVectorListSize], this->m_gui_dirContentInfo.fileList.at(i).name.c_str(), this->m_gui_dirContentInfo.fileList.at(i).name.size());
	}

	std::cout<<"path_size : "<<m_dirVectorListSize+m_fileVectorListSize<<std::endl;

	std::cout<<"path_real_size : "<<sizeof(this->mptr_gui_path_list)/sizeof(this->mptr_gui_path_list[0])<<std::endl;
}
#endif

// This is for display files and folders.
#if 0
if (ImGui::BeginTable("table1", 3, flags))
{
    // Display headers so we can inspect their interaction with borders.
    // (Headers are not the main purpose of this section of the demo, so we are not elaborating on them too much. See other sections for details)
    if (display_headers)
    {
        ImGui::TableSetupColumn("One");
        ImGui::TableSetupColumn("Two");
        ImGui::TableSetupColumn("Three");
        ImGui::TableHeadersRow();
    }

    for (int row = 0; row < 5; row++)
    {
        ImGui::TableNextRow();
        for (int column = 0; column < 3; column++)
        {
            ImGui::TableSetColumnIndex(column);
            char buf[32];
            sprintf(buf, "Hello %d,%d", column, row);
            if (contents_type == CT_Text)
                ImGui::TextUnformatted(buf);
            else if (contents_type == CT_FillButton)
                ImGui::Button(buf, ImVec2(-FLT_MIN, 0.0f));
        }
    }
    ImGui::EndTable();

#endif


// This is for directory selectable list box// need button
#if 0
// BeginListBox() is essentially a thin wrapper to using BeginChild()/EndChild() with the ImGuiChildFlags_FrameStyle flag for stylistic changes + displaying a label.
// You may be tempted to simply use BeginChild() directly, however note that BeginChild() requires EndChild() to always be called (inconsistent with BeginListBox()/EndListBox()).

// Using the generic BeginListBox() API, you have full control over how to display the combo contents.
// (your selection data could be an index, a pointer to the object, an id for the object, a flag intrusively
// stored in the object itself, etc.)
const char* items[] = { "AAAA", "BBBB", "CCCC", "DDDD", "EEEE", "FFFF", "GGGG", "HHHH", "IIII", "JJJJ", "KKKK", "LLLLLLL", "MMMM", "OOOOOOO" };
static int item_current_idx = 0; // Here we store our selection data as an index.
if (ImGui::BeginListBox("listbox 1"))
{
    for (int n = 0; n < IM_ARRAYSIZE(items); n++)
    {
        const bool is_selected = (item_current_idx == n);
        if (ImGui::Selectable(items[n], is_selected))
            item_current_idx = n;

        // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
        if (is_selected)
            ImGui::SetItemDefaultFocus();
    }
    ImGui::EndListBox();
}

#endif
#if 0
//-----------------------------------------------------------------------------
// [SECTION] Example App: Simple Layout / ShowExampleAppLayout()
//-----------------------------------------------------------------------------

// Demonstrate create a window with multiple child windows.
static void ShowExampleAppLayout(bool* p_open)
{
    ImGui::SetNextWindowSize(ImVec2(500, 440), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Example: Simple layout", p_open, ImGuiWindowFlags_MenuBar))
    {
        IMGUI_DEMO_MARKER("Examples/Simple layout");
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Close", "Ctrl+W")) { *p_open = false; }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        // Left
        static int selected = 0;
        {
            ImGui::BeginChild("left pane", ImVec2(150, 0), ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX);
            for (int i = 0; i < 100; i++)
            {
                // FIXME: Good candidate to use ImGuiSelectableFlags_SelectOnNav
                char label[128];
                sprintf(label, "MyObject %d", i);
                if (ImGui::Selectable(label, selected == i))
                    selected = i;
            }
            ImGui::EndChild();
        }
        ImGui::SameLine();

        // Right
        {
            ImGui::BeginGroup();
            ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us
            ImGui::Text("MyObject: %d", selected);
            ImGui::Separator();
            if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None))
            {
                if (ImGui::BeginTabItem("Description"))
                {
                    ImGui::TextWrapped("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. ");
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Details"))
                {
                    ImGui::Text("ID: 0123456789");
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            ImGui::EndChild();
            if (ImGui::Button("Revert")) {}
            ImGui::SameLine();
            if (ImGui::Button("Save")) {}
            ImGui::EndGroup();
        }
    }
    ImGui::End();
}


#endif
#if 0
struct ExampleAppConsole
{
    char                  InputBuf[256];
    ImVector<char*>       Items;
    ImVector<const char*> Commands;
    ImVector<char*>       History;
    int                   HistoryPos;    // -1: new line, 0..History.Size-1 browsing history.
    ImGuiTextFilter       Filter;
    bool                  AutoScroll;
    bool                  ScrollToBottom;

    ExampleAppConsole()
    {
        IMGUI_DEMO_MARKER("Examples/Console");
        ClearLog();
        memset(InputBuf, 0, sizeof(InputBuf));
        HistoryPos = -1;

        // "CLASSIFY" is here to provide the test case where "C"+[tab] completes to "CL" and display multiple matches.
        Commands.push_back("HELP");
        Commands.push_back("HISTORY");
        Commands.push_back("CLEAR");
        Commands.push_back("CLASSIFY");
        AutoScroll = true;
        ScrollToBottom = false;
        AddLog("Welcome to Dear ImGui!");
    }
    ~ExampleAppConsole()
    {
        ClearLog();
        for (int i = 0; i < History.Size; i++)
            free(History[i]);
    }

    // Portable helpers
    static int   Stricmp(const char* s1, const char* s2)         { int d; while ((d = toupper(*s2) - toupper(*s1)) == 0 && *s1) { s1++; s2++; } return d; }
    static int   Strnicmp(const char* s1, const char* s2, int n) { int d = 0; while (n > 0 && (d = toupper(*s2) - toupper(*s1)) == 0 && *s1) { s1++; s2++; n--; } return d; }
    static char* Strdup(const char* s)                           { IM_ASSERT(s); size_t len = strlen(s) + 1; void* buf = malloc(len); IM_ASSERT(buf); return (char*)memcpy(buf, (const void*)s, len); }
    static void  Strtrim(char* s)                                { char* str_end = s + strlen(s); while (str_end > s && str_end[-1] == ' ') str_end--; *str_end = 0; }

    void    ClearLog()
    {
        for (int i = 0; i < Items.Size; i++)
            free(Items[i]);
        Items.clear();
    }

    void    AddLog(const char* fmt, ...) IM_FMTARGS(2)
    {
        // FIXME-OPT
        char buf[1024];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
        buf[IM_ARRAYSIZE(buf)-1] = 0;
        va_end(args);
        Items.push_back(Strdup(buf));
    }

    void    Draw(const char* title, bool* p_open)
    {
        ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin(title, p_open))
        {
            ImGui::End();
            return;
        }

        // As a specific feature guaranteed by the library, after calling Begin() the last Item represent the title bar.
        // So e.g. IsItemHovered() will return true when hovering the title bar.
        // Here we create a context menu only available from the title bar.
        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem("Close Console"))
                *p_open = false;
            ImGui::EndPopup();
        }

        ImGui::TextWrapped(
            "This example implements a console with basic coloring, completion (TAB key) and history (Up/Down keys). A more elaborate "
            "implementation may want to store entries along with extra data such as timestamp, emitter, etc.");
        ImGui::TextWrapped("Enter 'HELP' for help.");

        // TODO: display items starting from the bottom

        if (ImGui::SmallButton("Add Debug Text"))  { AddLog("%d some text", Items.Size); AddLog("some more text"); AddLog("display very important message here!"); }
        ImGui::SameLine();
        if (ImGui::SmallButton("Add Debug Error")) { AddLog("[error] something went wrong"); }
        ImGui::SameLine();
        if (ImGui::SmallButton("Clear"))           { ClearLog(); }
        ImGui::SameLine();
        bool copy_to_clipboard = ImGui::SmallButton("Copy");
        //static float t = 0.0f; if (ImGui::GetTime() - t > 0.02f) { t = ImGui::GetTime(); AddLog("Spam %f", t); }

        ImGui::Separator();

        // Options menu
        if (ImGui::BeginPopup("Options"))
        {
            ImGui::Checkbox("Auto-scroll", &AutoScroll);
            ImGui::EndPopup();
        }

        // Options, Filter
        if (ImGui::Button("Options"))
            ImGui::OpenPopup("Options");
        ImGui::SameLine();
        Filter.Draw("Filter (\"incl,-excl\") (\"error\")", 180);
        ImGui::Separator();

        // Reserve enough left-over height for 1 separator + 1 input text
        const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
        if (ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar))
        {
            if (ImGui::BeginPopupContextWindow())
            {
                if (ImGui::Selectable("Clear")) ClearLog();
                ImGui::EndPopup();
            }

            // Display every line as a separate entry so we can change their color or add custom widgets.
            // If you only want raw text you can use ImGui::TextUnformatted(log.begin(), log.end());
            // NB- if you have thousands of entries this approach may be too inefficient and may require user-side clipping
            // to only process visible items. The clipper will automatically measure the height of your first item and then
            // "seek" to display only items in the visible area.
            // To use the clipper we can replace your standard loop:
            //      for (int i = 0; i < Items.Size; i++)
            //   With:
            //      ImGuiListClipper clipper;
            //      clipper.Begin(Items.Size);
            //      while (clipper.Step())
            //         for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
            // - That your items are evenly spaced (same height)
            // - That you have cheap random access to your elements (you can access them given their index,
            //   without processing all the ones before)
            // You cannot this code as-is if a filter is active because it breaks the 'cheap random-access' property.
            // We would need random-access on the post-filtered list.
            // A typical application wanting coarse clipping and filtering may want to pre-compute an array of indices
            // or offsets of items that passed the filtering test, recomputing this array when user changes the filter,
            // and appending newly elements as they are inserted. This is left as a task to the user until we can manage
            // to improve this example code!
            // If your items are of variable height:
            // - Split them into same height items would be simpler and facilitate random-seeking into your list.
            // - Consider using manual call to IsRectVisible() and skipping extraneous decoration from your items.
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
            if (copy_to_clipboard)
                ImGui::LogToClipboard();
            for (const char* item : Items)
            {
                if (!Filter.PassFilter(item))
                    continue;

                // Normally you would store more information in your item than just a string.
                // (e.g. make Items[] an array of structure, store color/type etc.)
                ImVec4 color;
                bool has_color = false;
                if (strstr(item, "[error]")) { color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f); has_color = true; }
                else if (strncmp(item, "# ", 2) == 0) { color = ImVec4(1.0f, 0.8f, 0.6f, 1.0f); has_color = true; }
                if (has_color)
                    ImGui::PushStyleColor(ImGuiCol_Text, color);
                ImGui::TextUnformatted(item);
                if (has_color)
                    ImGui::PopStyleColor();
            }
            if (copy_to_clipboard)
                ImGui::LogFinish();

            // Keep up at the bottom of the scroll region if we were already at the bottom at the beginning of the frame.
            // Using a scrollbar or mouse-wheel will take away from the bottom edge.
            if (ScrollToBottom || (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
                ImGui::SetScrollHereY(1.0f);
            ScrollToBottom = false;

            ImGui::PopStyleVar();
        }
        ImGui::EndChild();
        ImGui::Separator();

        // Command-line
        bool reclaim_focus = false;
        ImGuiInputTextFlags input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory;
        if (ImGui::InputText("Input", InputBuf, IM_ARRAYSIZE(InputBuf), input_text_flags, &TextEditCallbackStub, (void*)this))
        {
            char* s = InputBuf;
            Strtrim(s);
            if (s[0])
                ExecCommand(s);
            strcpy(s, "");
            reclaim_focus = true;
        }

        // Auto-focus on window apparition
        ImGui::SetItemDefaultFocus();
        if (reclaim_focus)
            ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget

        ImGui::End();
    }

    void    ExecCommand(const char* command_line)
    {
        AddLog("# %s\n", command_line);

        // Insert into history. First find match and delete it so it can be pushed to the back.
        // This isn't trying to be smart or optimal.
        HistoryPos = -1;
        for (int i = History.Size - 1; i >= 0; i--)
            if (Stricmp(History[i], command_line) == 0)
            {
                free(History[i]);
                History.erase(History.begin() + i);
                break;
            }
        History.push_back(Strdup(command_line));

        // Process command
        if (Stricmp(command_line, "CLEAR") == 0)
        {
            ClearLog();
        }
        else if (Stricmp(command_line, "HELP") == 0)
        {
            AddLog("Commands:");
            for (int i = 0; i < Commands.Size; i++)
                AddLog("- %s", Commands[i]);
        }
        else if (Stricmp(command_line, "HISTORY") == 0)
        {
            int first = History.Size - 10;
            for (int i = first > 0 ? first : 0; i < History.Size; i++)
                AddLog("%3d: %s\n", i, History[i]);
        }
        else
        {
            AddLog("Unknown command: '%s'\n", command_line);
        }

        // On command input, we scroll to bottom even if AutoScroll==false
        ScrollToBottom = true;
    }

    // In C++11 you'd be better off using lambdas for this sort of forwarding callbacks
    static int TextEditCallbackStub(ImGuiInputTextCallbackData* data)
    {
        ExampleAppConsole* console = (ExampleAppConsole*)data->UserData;
        return console->TextEditCallback(data);
    }

    int     TextEditCallback(ImGuiInputTextCallbackData* data)
    {
        //AddLog("cursor: %d, selection: %d-%d", data->CursorPos, data->SelectionStart, data->SelectionEnd);
        switch (data->EventFlag)
        {
        case ImGuiInputTextFlags_CallbackCompletion:
            {
                // Example of TEXT COMPLETION

                // Locate beginning of current word
                const char* word_end = data->Buf + data->CursorPos;
                const char* word_start = word_end;
                while (word_start > data->Buf)
                {
                    const char c = word_start[-1];
                    if (c == ' ' || c == '\t' || c == ',' || c == ';')
                        break;
                    word_start--;
                }

                // Build a list of candidates
                ImVector<const char*> candidates;
                for (int i = 0; i < Commands.Size; i++)
                    if (Strnicmp(Commands[i], word_start, (int)(word_end - word_start)) == 0)
                        candidates.push_back(Commands[i]);

                if (candidates.Size == 0)
                {
                    // No match
                    AddLog("No match for \"%.*s\"!\n", (int)(word_end - word_start), word_start);
                }
                else if (candidates.Size == 1)
                {
                    // Single match. Delete the beginning of the word and replace it entirely so we've got nice casing.
                    data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
                    data->InsertChars(data->CursorPos, candidates[0]);
                    data->InsertChars(data->CursorPos, " ");
                }
                else
                {
                    // Multiple matches. Complete as much as we can..
                    // So inputing "C"+Tab will complete to "CL" then display "CLEAR" and "CLASSIFY" as matches.
                    int match_len = (int)(word_end - word_start);
                    for (;;)
                    {
                        int c = 0;
                        bool all_candidates_matches = true;
                        for (int i = 0; i < candidates.Size && all_candidates_matches; i++)
                            if (i == 0)
                                c = toupper(candidates[i][match_len]);
                            else if (c == 0 || c != toupper(candidates[i][match_len]))
                                all_candidates_matches = false;
                        if (!all_candidates_matches)
                            break;
                        match_len++;
                    }

                    if (match_len > 0)
                    {
                        data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
                        data->InsertChars(data->CursorPos, candidates[0], candidates[0] + match_len);
                    }

                    // List matches
                    AddLog("Possible matches:\n");
                    for (int i = 0; i < candidates.Size; i++)
                        AddLog("- %s\n", candidates[i]);
                }

                break;
            }
        case ImGuiInputTextFlags_CallbackHistory:
            {
                // Example of HISTORY
                const int prev_history_pos = HistoryPos;
                if (data->EventKey == ImGuiKey_UpArrow)
                {
                    if (HistoryPos == -1)
                        HistoryPos = History.Size - 1;
                    else if (HistoryPos > 0)
                        HistoryPos--;
                }
                else if (data->EventKey == ImGuiKey_DownArrow)
                {
                    if (HistoryPos != -1)
                        if (++HistoryPos >= History.Size)
                            HistoryPos = -1;
                }

                // A better implementation would preserve the data on the current input line along with cursor position.
                if (prev_history_pos != HistoryPos)
                {
                    const char* history_str = (HistoryPos >= 0) ? History[HistoryPos] : "";
                    data->DeleteChars(0, data->BufTextLen);
                    data->InsertChars(0, history_str);
                }
            }
        }
        return 0;
    }
};

static void ShowExampleAppConsole(bool* p_open)
{
    static ExampleAppConsole console; //AppConsole Class
    console.Draw("Example: Console", p_open);
}

#endif
