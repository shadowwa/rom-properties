/******************************************************************************
 * ROM Properties Page shell extension. (libwin32common)                      *
 * IListView.hpp: IListView interfaces. (undocumented)                        *
 *                                                                            *
 * Based on the Undocumented List View Features tutorial on CodeProject:      *
 * https://www.codeproject.com/Articles/35197/Undocumented-List-View-Features *
 ******************************************************************************/

#ifndef __ROMPROPERTIES_LIBWIN32COMMON_SDK_ILISTVIEW_HPP__
#define __ROMPROPERTIES_LIBWIN32COMMON_SDK_ILISTVIEW_HPP__

#include "../RpWin32_sdk.h"
#include <oleidl.h>
#include <commctrl.h>

// Windows Vista structs and macros.
#if _WIN32_WINNT < 0x0600
// LVGROUP for Vista.
typedef struct LVGROUP_Vista {
	UINT   cbSize;
	UINT   mask;
	LPWSTR pszHeader;
	int    cchHeader;
	LPWSTR pszFooter;
	int    cchFooter;
	int    iGroupId;
	UINT   stateMask;
	UINT   state;
	UINT   uAlign;

	// Vista-specific fields.
	LPWSTR pszSubtitle;
	UINT   cchSubtitle;
	LPWSTR pszTask;
	UINT   cchTask;
	LPWSTR pszDescriptionTop;
	UINT   cchDescriptionTop;
	LPWSTR pszDescriptionBottom;
	UINT   cchDescriptionBottom;
	int    iTitleImage;
	int    iExtendedImage;
	int    iFirstItem;
	UINT   cItems;
	LPWSTR pszSubsetTitle;
	UINT   cchSubsetTitle;
} LVGROUP_Vista, *PLVGROUP_Vista;

// LVITEMINDEX
typedef struct tagLVITEMINDEX {
	int iItem;
	int iGroup;
} LVITEMINDEX, *PLVITEMINDEX;

#define LVGF_SUBTITLE           0x00000100  // pszSubtitle is valid
#define LVGF_TASK               0x00000200  // pszTask is valid
#define LVGF_DESCRIPTIONTOP     0x00000400  // pszDescriptionTop is valid
#define LVGF_DESCRIPTIONBOTTOM  0x00000800  // pszDescriptionBottom is valid
#define LVGF_TITLEIMAGE         0x00001000  // iTitleImage is valid
#define LVGF_EXTENDEDIMAGE      0x00002000  // iExtendedImage is valid
#define LVGF_ITEMS              0x00004000  // iFirstItem and cItems are valid
#define LVGF_SUBSET             0x00008000  // pszSubsetTitle is valid
#define LVGF_SUBSETITEMS        0x00010000  // readonly, cItems holds count of items in visible subset, iFirstItem is valid

#else /* !(_WIN32_WINNT < 0x0600) */

// TODO: Verify that this works.
typedef LVGROUP LVGROUP_Vista;
typedef PLVGROUP PLVGROUP_Vista;

#endif /* _WIN32_WINNT < 0x0600 */

// Interface IDs.
extern "C" {
	extern const IID IID_IListView_WinVista;
	extern const IID IID_IListView_Win7;
};

// ListView message to get the IListView interface.
#define LVM_QUERYINTERFACE (LVM_FIRST + 189)
#define ListView_QueryInterface(hWnd,riid,pOut) (void)SNDMSG((hWnd),LVM_QUERYINTERFACE,(WPARAM)&(riid),(LPARAM)(pOut))

class IOwnerDataCallback;

class UUID_ATTR("{2FFE2979-5928-4386-9CDB-8E1F15B72FB4}")
IListView_WinVista : public IOleWindow
{
	public:
		virtual HRESULT STDMETHODCALLTYPE GetImageList(int imageList, HIMAGELIST* pHImageList) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetImageList(int imageList, HIMAGELIST hNewImageList, HIMAGELIST* pHOldImageList) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetBackgroundColor(COLORREF* pColor) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetBackgroundColor(COLORREF color) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetTextColor(COLORREF* pColor) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetTextColor(COLORREF color) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetTextBackgroundColor(COLORREF* pColor) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetTextBackgroundColor(COLORREF color) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetHotLightColor(COLORREF* pColor) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetHotLightColor(COLORREF color) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetItemCount(PINT pItemCount) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetItemCount(int itemCount, DWORD flags) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetItem(LVITEMW* pItem) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetItem(LVITEMW* const pItem) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetItemState(int itemIndex, int subItemIndex, ULONG mask, ULONG* pState) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetItemState(int itemIndex, int subItemIndex, ULONG mask, ULONG state) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetItemText(int itemIndex, int subItemIndex, LPWSTR pBuffer, int bufferSize) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetItemText(int itemIndex, int subItemIndex, LPCWSTR pText) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetBackgroundImage(LVBKIMAGEW* pBkImage) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetBackgroundImage(LVBKIMAGEW* const pBkImage) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetFocusedColumn(PINT pColumnIndex) = 0;
		// parameters may be in wrong order
		virtual HRESULT STDMETHODCALLTYPE SetSelectionFlags(ULONG mask, ULONG flags) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetSelectedColumn(PINT pColumnIndex) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetSelectedColumn(int columnIndex) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetView(DWORD* pView) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetView(DWORD view) = 0;
		virtual HRESULT STDMETHODCALLTYPE InsertItem(LVITEMW* const pItem, PINT pItemIndex) = 0;
		virtual HRESULT STDMETHODCALLTYPE DeleteItem(int itemIndex) = 0;
		virtual HRESULT STDMETHODCALLTYPE DeleteAllItems(void) = 0;
		virtual HRESULT STDMETHODCALLTYPE UpdateItem(int itemIndex) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetItemRect(LVITEMINDEX itemIndex, int rectangleType, LPRECT pRectangle) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetSubItemRect(LVITEMINDEX itemIndex, int subItemIndex, int rectangleType, LPRECT pRectangle) = 0;
		virtual HRESULT STDMETHODCALLTYPE HitTestSubItem(LVHITTESTINFO* pHitTestData) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetIncrSearchString(PWSTR pBuffer, int bufferSize, PINT pCopiedChars) = 0;
		// pHorizontalSpacing and pVerticalSpacing may be in wrong order
		virtual HRESULT STDMETHODCALLTYPE GetItemSpacing(BOOL smallIconView, PINT pHorizontalSpacing, PINT pVerticalSpacing) = 0;
		// parameters may be in wrong order
		virtual HRESULT STDMETHODCALLTYPE SetIconSpacing(int horizontalSpacing, int verticalSpacing, PINT pHorizontalSpacing, PINT pVerticalSpacing) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetNextItem(LVITEMINDEX itemIndex, ULONG flags, LVITEMINDEX* pNextItemIndex) = 0;
		virtual HRESULT STDMETHODCALLTYPE FindItem(LVITEMINDEX startItemIndex, LVFINDINFOW const* pFindInfo, LVITEMINDEX* pFoundItemIndex) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetSelectionMark(LVITEMINDEX* pSelectionMark) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetSelectionMark(LVITEMINDEX newSelectionMark, LVITEMINDEX* pOldSelectionMark) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetItemPosition(LVITEMINDEX itemIndex, POINT* pPosition) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetItemPosition(int itemIndex, POINT const* pPosition) = 0;
		// parameters may be in wrong order
		virtual HRESULT STDMETHODCALLTYPE ScrollView(int horizontalScrollDistance, int verticalScrollDistance) = 0;
		virtual HRESULT STDMETHODCALLTYPE EnsureItemVisible(LVITEMINDEX itemIndex, BOOL partialOk) = 0;
		virtual HRESULT STDMETHODCALLTYPE EnsureSubItemVisible(LVITEMINDEX itemIndex, int subItemIndex) = 0;
		virtual HRESULT STDMETHODCALLTYPE EditSubItem(LVITEMINDEX itemIndex, int subItemIndex) = 0;
		virtual HRESULT STDMETHODCALLTYPE RedrawItems(int firstItemIndex, int lastItemIndex) = 0;
		virtual HRESULT STDMETHODCALLTYPE ArrangeItems(int mode) = 0;
		virtual HRESULT STDMETHODCALLTYPE RecomputeItems(int unknown) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetEditControl(HWND* pHWndEdit) = 0;
		// TODO: verify that 'initialEditText' really is used to specify the initial text
		virtual HRESULT STDMETHODCALLTYPE EditLabel(LVITEMINDEX itemIndex, LPCWSTR initialEditText, HWND* phWndEdit) = 0;
		virtual HRESULT STDMETHODCALLTYPE EditGroupLabel(int groupIndex) = 0;
		virtual HRESULT STDMETHODCALLTYPE CancelEditLabel(void) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetEditItem(LVITEMINDEX* itemIndex, PINT subItemIndex) = 0;
		virtual HRESULT STDMETHODCALLTYPE HitTest(LVHITTESTINFO* pHitTestData) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetStringWidth(PCWSTR pString, PINT pWidth) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetColumn(int columnIndex, LVCOLUMNW* pColumn) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetColumn(int columnIndex, LVCOLUMNW* const pColumn) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetColumnOrderArray(int numberOfColumns, PINT pColumns) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetColumnOrderArray(int numberOfColumns, int const* pColumns) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetHeaderControl(HWND* pHWndHeader) = 0;
		virtual HRESULT STDMETHODCALLTYPE InsertColumn(int insertAt, LVCOLUMNW* const pColumn, PINT pColumnIndex) = 0;
		virtual HRESULT STDMETHODCALLTYPE DeleteColumn(int columnIndex) = 0;
		virtual HRESULT STDMETHODCALLTYPE CreateDragImage(int itemIndex, POINT const* pUpperLeft, HIMAGELIST* pHImageList) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetViewRect(RECT* pRectangle) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetClientRect(BOOL unknown, RECT* pClientRectangle) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetColumnWidth(int columnIndex, PINT pWidth) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetColumnWidth(int columnIndex, int width) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetCallbackMask(ULONG* pMask) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetCallbackMask(ULONG mask) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetTopIndex(PINT pTopIndex) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetCountPerPage(PINT pCountPerPage) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetOrigin(POINT* pOrigin) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetSelectedCount(PINT pSelectedCount) = 0;
		// 'unknown' might specify whether to pass items' data or indexes
		virtual HRESULT STDMETHODCALLTYPE SortItems(BOOL unknown, LPARAM lParam, PFNLVCOMPARE pComparisonFunction) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetExtendedStyle(DWORD* pStyle) = 0;
		// parameters may be in wrong order
		virtual HRESULT STDMETHODCALLTYPE SetExtendedStyle(DWORD mask, DWORD style, DWORD* pOldStyle) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetHoverTime(UINT* pTime) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetHoverTime(UINT time, UINT* pOldSetting) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetToolTip(HWND* pHWndToolTip) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetToolTip(HWND hWndToolTip, HWND* pHWndOldToolTip) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetHotItem(LVITEMINDEX* pHotItem) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetHotItem(LVITEMINDEX newHotItem, LVITEMINDEX* pOldHotItem) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetHotCursor(HCURSOR* pHCursor) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetHotCursor(HCURSOR hCursor, HCURSOR* pHOldCursor) = 0;
		// parameters may be in wrong order
		virtual HRESULT STDMETHODCALLTYPE ApproximateViewRect(int itemCount, PINT pWidth, PINT pHeight) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetRangeObject(int unknown, LPVOID/*ILVRange**/ pObject) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetWorkAreas(int numberOfWorkAreas, RECT* pWorkAreas) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetWorkAreas(int numberOfWorkAreas, RECT const* pWorkAreas) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetWorkAreaCount(PINT pNumberOfWorkAreas) = 0;
		virtual HRESULT STDMETHODCALLTYPE ResetEmptyText(void) = 0;
		virtual HRESULT STDMETHODCALLTYPE EnableGroupView(BOOL enable) = 0;
		virtual HRESULT STDMETHODCALLTYPE IsGroupViewEnabled(BOOL* pIsEnabled) = 0;
		virtual HRESULT STDMETHODCALLTYPE SortGroups(PFNLVGROUPCOMPARE pComparisonFunction, PVOID lParam) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetGroupInfo(int unknown1, int unknown2, LVGROUP* pGroup) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetGroupInfo(int unknown, int groupID, LVGROUP* const pGroup) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetGroupRect(BOOL unknown, int groupID, int rectangleType, RECT* pRectangle) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetGroupState(int groupID, ULONG mask, ULONG* pState) = 0;
		virtual HRESULT STDMETHODCALLTYPE HasGroup(int groupID, BOOL* pHasGroup) = 0;
		virtual HRESULT STDMETHODCALLTYPE InsertGroup(int insertAt, LVGROUP* const pGroup, PINT pGroupID) = 0;
		virtual HRESULT STDMETHODCALLTYPE RemoveGroup(int groupID) = 0;
		virtual HRESULT STDMETHODCALLTYPE InsertGroupSorted(LVINSERTGROUPSORTED const* pGroup, PINT pGroupID) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetGroupMetrics(LVGROUPMETRICS* pMetrics) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetGroupMetrics(LVGROUPMETRICS* const pMetrics) = 0;
		virtual HRESULT STDMETHODCALLTYPE RemoveAllGroups(void) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetFocusedGroup(PINT pGroupID) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetGroupCount(PINT pCount) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetOwnerDataCallback(IOwnerDataCallback* pCallback) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetTileViewInfo(LVTILEVIEWINFO* pInfo) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetTileViewInfo(LVTILEVIEWINFO* const pInfo) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetTileInfo(LVTILEINFO* pTileInfo) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetTileInfo(LVTILEINFO* const pTileInfo) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetInsertMark(LVINSERTMARK* pInsertMarkDetails) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetInsertMark(LVINSERTMARK const* pInsertMarkDetails) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetInsertMarkRect(LPRECT pInsertMarkRectangle) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetInsertMarkColor(COLORREF* pColor) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetInsertMarkColor(COLORREF color, COLORREF* pOldColor) = 0;
		virtual HRESULT STDMETHODCALLTYPE HitTestInsertMark(POINT const* pPoint, LVINSERTMARK* pInsertMarkDetails) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetInfoTip(LVSETINFOTIP* const pInfoTip) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetOutlineColor(COLORREF* pColor) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetOutlineColor(COLORREF color, COLORREF* pOldColor) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetFrozenItem(PINT pItemIndex) = 0;
		// one parameter will be the item index; works in Icons view only
		virtual HRESULT STDMETHODCALLTYPE SetFrozenItem(int unknown1, int unknown2) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetFrozenSlot(RECT* pUnknown) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetFrozenSlot(int unknown1, POINT const* pUnknown2) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetViewMargin(RECT* pMargin) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetViewMargin(RECT const* pMargin) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetKeyboardSelected(LVITEMINDEX itemIndex) = 0;
		virtual HRESULT STDMETHODCALLTYPE MapIndexToId(int itemIndex, PINT pItemID) = 0;
		virtual HRESULT STDMETHODCALLTYPE MapIdToIndex(int itemID, PINT pItemIndex) = 0;
		virtual HRESULT STDMETHODCALLTYPE IsItemVisible(LVITEMINDEX itemIndex, BOOL* pVisible) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetGroupSubsetCount(PINT pNumberOfRowsDisplayed) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetGroupSubsetCount(int numberOfRowsToDisplay) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetVisibleSlotCount(PINT pCount) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetColumnMargin(RECT* pMargin) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetSubItemCallback(LPVOID/*ISubItemCallback**/ pCallback) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetVisibleItemRange(LVITEMINDEX* pFirstItem, LVITEMINDEX* pLastItem) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetTypeAheadFlags(UINT mask, UINT flags) = 0;
};

#ifdef __CRT_UUID_DECL
// Required for MinGW-w64 __uuidof() emulation.
__CRT_UUID_DECL(IListView_WinVista, 0x2FFE2979, 0x5928, 0x4386, 0x9C, 0xDB, 0x8E, 0x1F, 0x15, 0xB7, 0x2F, 0xB4)
#endif

class UUID_ATTR("{E5B16AF2-3990-4681-A609-1F060CD14269}")
IListView_Win7 : public IOleWindow
{
	public:
		virtual HRESULT STDMETHODCALLTYPE GetImageList(int imageList, HIMAGELIST* pHImageList) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetImageList(int imageList, HIMAGELIST hNewImageList, HIMAGELIST* pHOldImageList) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetBackgroundColor(COLORREF* pColor) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetBackgroundColor(COLORREF color) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetTextColor(COLORREF* pColor) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetTextColor(COLORREF color) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetTextBackgroundColor(COLORREF* pColor) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetTextBackgroundColor(COLORREF color) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetHotLightColor(COLORREF* pColor) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetHotLightColor(COLORREF color) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetItemCount(PINT pItemCount) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetItemCount(int itemCount, DWORD flags) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetItem(LVITEMW* pItem) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetItem(LVITEMW* const pItem) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetItemState(int itemIndex, int subItemIndex, ULONG mask, ULONG* pState) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetItemState(int itemIndex, int subItemIndex, ULONG mask, ULONG state) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetItemText(int itemIndex, int subItemIndex, LPWSTR pBuffer, int bufferSize) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetItemText(int itemIndex, int subItemIndex, LPCWSTR pText) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetBackgroundImage(LVBKIMAGEW* pBkImage) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetBackgroundImage(LVBKIMAGEW* const pBkImage) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetFocusedColumn(PINT pColumnIndex) = 0;
		// parameters may be in wrong order
		virtual HRESULT STDMETHODCALLTYPE SetSelectionFlags(ULONG mask, ULONG flags) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetSelectedColumn(PINT pColumnIndex) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetSelectedColumn(int columnIndex) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetView(DWORD* pView) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetView(DWORD view) = 0;
		virtual HRESULT STDMETHODCALLTYPE InsertItem(LVITEMW* const pItem, PINT pItemIndex) = 0;
		virtual HRESULT STDMETHODCALLTYPE DeleteItem(int itemIndex) = 0;
		virtual HRESULT STDMETHODCALLTYPE DeleteAllItems(void) = 0;
		virtual HRESULT STDMETHODCALLTYPE UpdateItem(int itemIndex) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetItemRect(LVITEMINDEX itemIndex, int rectangleType, LPRECT pRectangle) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetSubItemRect(LVITEMINDEX itemIndex, int subItemIndex, int rectangleType, LPRECT pRectangle) = 0;
		virtual HRESULT STDMETHODCALLTYPE HitTestSubItem(LVHITTESTINFO* pHitTestData) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetIncrSearchString(PWSTR pBuffer, int bufferSize, PINT pCopiedChars) = 0;
		// pHorizontalSpacing and pVerticalSpacing may be in wrong order
		virtual HRESULT STDMETHODCALLTYPE GetItemSpacing(BOOL smallIconView, PINT pHorizontalSpacing, PINT pVerticalSpacing) = 0;
		// parameters may be in wrong order
		virtual HRESULT STDMETHODCALLTYPE SetIconSpacing(int horizontalSpacing, int verticalSpacing, PINT pHorizontalSpacing, PINT pVerticalSpacing) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetNextItem(LVITEMINDEX itemIndex, ULONG flags, LVITEMINDEX* pNextItemIndex) = 0;
		virtual HRESULT STDMETHODCALLTYPE FindItem(LVITEMINDEX startItemIndex, LVFINDINFOW const* pFindInfo, LVITEMINDEX* pFoundItemIndex) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetSelectionMark(LVITEMINDEX* pSelectionMark) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetSelectionMark(LVITEMINDEX newSelectionMark, LVITEMINDEX* pOldSelectionMark) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetItemPosition(LVITEMINDEX itemIndex, POINT* pPosition) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetItemPosition(int itemIndex, POINT const* pPosition) = 0;
		// parameters may be in wrong order
		virtual HRESULT STDMETHODCALLTYPE ScrollView(int horizontalScrollDistance, int verticalScrollDistance) = 0;
		virtual HRESULT STDMETHODCALLTYPE EnsureItemVisible(LVITEMINDEX itemIndex, BOOL partialOk) = 0;
		virtual HRESULT STDMETHODCALLTYPE EnsureSubItemVisible(LVITEMINDEX itemIndex, int subItemIndex) = 0;
		virtual HRESULT STDMETHODCALLTYPE EditSubItem(LVITEMINDEX itemIndex, int subItemIndex) = 0;
		virtual HRESULT STDMETHODCALLTYPE RedrawItems(int firstItemIndex, int lastItemIndex) = 0;
		virtual HRESULT STDMETHODCALLTYPE ArrangeItems(int mode) = 0;
		virtual HRESULT STDMETHODCALLTYPE RecomputeItems(int unknown) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetEditControl(HWND* pHWndEdit) = 0;
		// TODO: verify that 'initialEditText' really is used to specify the initial text
		virtual HRESULT STDMETHODCALLTYPE EditLabel(LVITEMINDEX itemIndex, LPCWSTR initialEditText, HWND* phWndEdit) = 0;
		virtual HRESULT STDMETHODCALLTYPE EditGroupLabel(int groupIndex) = 0;
		virtual HRESULT STDMETHODCALLTYPE CancelEditLabel(void) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetEditItem(LVITEMINDEX* itemIndex, PINT subItemIndex) = 0;
		virtual HRESULT STDMETHODCALLTYPE HitTest(LVHITTESTINFO* pHitTestData) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetStringWidth(PCWSTR pString, PINT pWidth) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetColumn(int columnIndex, LVCOLUMNW* pColumn) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetColumn(int columnIndex, LVCOLUMNW* const pColumn) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetColumnOrderArray(int numberOfColumns, PINT pColumns) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetColumnOrderArray(int numberOfColumns, int const* pColumns) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetHeaderControl(HWND* pHWndHeader) = 0;
		virtual HRESULT STDMETHODCALLTYPE InsertColumn(int insertAt, LVCOLUMNW* const pColumn, PINT pColumnIndex) = 0;
		virtual HRESULT STDMETHODCALLTYPE DeleteColumn(int columnIndex) = 0;
		virtual HRESULT STDMETHODCALLTYPE CreateDragImage(int itemIndex, POINT const* pUpperLeft, HIMAGELIST* pHImageList) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetViewRect(RECT* pRectangle) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetClientRect(BOOL unknown, RECT* pClientRectangle) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetColumnWidth(int columnIndex, PINT pWidth) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetColumnWidth(int columnIndex, int width) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetCallbackMask(ULONG* pMask) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetCallbackMask(ULONG mask) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetTopIndex(PINT pTopIndex) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetCountPerPage(PINT pCountPerPage) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetOrigin(POINT* pOrigin) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetSelectedCount(PINT pSelectedCount) = 0;
		// 'unknown' might specify whether to pass items' data or indexes
		virtual HRESULT STDMETHODCALLTYPE SortItems(BOOL unknown, LPARAM lParam, PFNLVCOMPARE pComparisonFunction) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetExtendedStyle(DWORD* pStyle) = 0;
		// parameters may be in wrong order
		virtual HRESULT STDMETHODCALLTYPE SetExtendedStyle(DWORD mask, DWORD style, DWORD* pOldStyle) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetHoverTime(UINT* pTime) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetHoverTime(UINT time, UINT* pOldSetting) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetToolTip(HWND* pHWndToolTip) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetToolTip(HWND hWndToolTip, HWND* pHWndOldToolTip) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetHotItem(LVITEMINDEX* pHotItem) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetHotItem(LVITEMINDEX newHotItem, LVITEMINDEX* pOldHotItem) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetHotCursor(HCURSOR* pHCursor) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetHotCursor(HCURSOR hCursor, HCURSOR* pHOldCursor) = 0;
		// parameters may be in wrong order
		virtual HRESULT STDMETHODCALLTYPE ApproximateViewRect(int itemCount, PINT pWidth, PINT pHeight) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetRangeObject(int unknown, LPVOID/*ILVRange**/ pObject) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetWorkAreas(int numberOfWorkAreas, RECT* pWorkAreas) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetWorkAreas(int numberOfWorkAreas, RECT const* pWorkAreas) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetWorkAreaCount(PINT pNumberOfWorkAreas) = 0;
		virtual HRESULT STDMETHODCALLTYPE ResetEmptyText(void) = 0;
		virtual HRESULT STDMETHODCALLTYPE EnableGroupView(BOOL enable) = 0;
		virtual HRESULT STDMETHODCALLTYPE IsGroupViewEnabled(BOOL* pIsEnabled) = 0;
		virtual HRESULT STDMETHODCALLTYPE SortGroups(PFNLVGROUPCOMPARE pComparisonFunction, PVOID lParam) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetGroupInfo(int unknown1, int unknown2, LVGROUP* pGroup) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetGroupInfo(int unknown, int groupID, LVGROUP* const pGroup) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetGroupRect(BOOL unknown, int groupID, int rectangleType, RECT* pRectangle) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetGroupState(int groupID, ULONG mask, ULONG* pState) = 0;
		virtual HRESULT STDMETHODCALLTYPE HasGroup(int groupID, BOOL* pHasGroup) = 0;
		virtual HRESULT STDMETHODCALLTYPE InsertGroup(int insertAt, LVGROUP* const pGroup, PINT pGroupID) = 0;
		virtual HRESULT STDMETHODCALLTYPE RemoveGroup(int groupID) = 0;
		virtual HRESULT STDMETHODCALLTYPE InsertGroupSorted(LVINSERTGROUPSORTED const* pGroup, PINT pGroupID) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetGroupMetrics(LVGROUPMETRICS* pMetrics) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetGroupMetrics(LVGROUPMETRICS* const pMetrics) = 0;
		virtual HRESULT STDMETHODCALLTYPE RemoveAllGroups(void) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetFocusedGroup(PINT pGroupID) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetGroupCount(PINT pCount) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetOwnerDataCallback(IOwnerDataCallback* pCallback) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetTileViewInfo(LVTILEVIEWINFO* pInfo) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetTileViewInfo(LVTILEVIEWINFO* const pInfo) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetTileInfo(LVTILEINFO* pTileInfo) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetTileInfo(LVTILEINFO* const pTileInfo) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetInsertMark(LVINSERTMARK* pInsertMarkDetails) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetInsertMark(LVINSERTMARK const* pInsertMarkDetails) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetInsertMarkRect(LPRECT pInsertMarkRectangle) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetInsertMarkColor(COLORREF* pColor) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetInsertMarkColor(COLORREF color, COLORREF* pOldColor) = 0;
		virtual HRESULT STDMETHODCALLTYPE HitTestInsertMark(POINT const* pPoint, LVINSERTMARK* pInsertMarkDetails) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetInfoTip(LVSETINFOTIP* const pInfoTip) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetOutlineColor(COLORREF* pColor) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetOutlineColor(COLORREF color, COLORREF* pOldColor) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetFrozenItem(PINT pItemIndex) = 0;
		// one parameter will be the item index; works in Icons view only
		virtual HRESULT STDMETHODCALLTYPE SetFrozenItem(int unknown1, int unknown2) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetFrozenSlot(RECT* pUnknown) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetFrozenSlot(int unknown1, POINT const* pUnknown2) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetViewMargin(RECT* pMargin) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetViewMargin(RECT const* pMargin) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetKeyboardSelected(LVITEMINDEX itemIndex) = 0;
		virtual HRESULT STDMETHODCALLTYPE MapIndexToId(int itemIndex, PINT pItemID) = 0;
		virtual HRESULT STDMETHODCALLTYPE MapIdToIndex(int itemID, PINT pItemIndex) = 0;
		virtual HRESULT STDMETHODCALLTYPE IsItemVisible(LVITEMINDEX itemIndex, BOOL* pVisible) = 0;
		virtual HRESULT STDMETHODCALLTYPE EnableAlphaShadow(BOOL enable) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetGroupSubsetCount(PINT pNumberOfRowsDisplayed) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetGroupSubsetCount(int numberOfRowsToDisplay) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetVisibleSlotCount(PINT pCount) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetColumnMargin(RECT* pMargin) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetSubItemCallback(LPVOID/*ISubItemCallback**/ pCallback) = 0;
		virtual HRESULT STDMETHODCALLTYPE GetVisibleItemRange(LVITEMINDEX* pFirstItem, LVITEMINDEX* pLastItem) = 0;
		virtual HRESULT STDMETHODCALLTYPE SetTypeAheadFlags(UINT mask, UINT flags) = 0;
};

#ifdef __CRT_UUID_DECL
// Required for MinGW-w64 __uuidof() emulation.
__CRT_UUID_DECL(IListView_Win7, 0xE5B16AF2, 0x3990, 0x4681, 0xA6, 0x09, 0x1F, 0x06, 0x0C, 0xD1, 0x42, 0x69)
#endif

#endif /* __ROMPROPERTIES_LIBWIN32COMMON_SDK_ILISTVIEW_HPP__ */
