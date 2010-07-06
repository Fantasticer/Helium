#pragma once

#include "Application/API.h"

#include <map>

#include <wx/animate.h>
#include <wx/artprov.h>
#include <wx/imaglist.h>
#include <wx/generic/dirctrlg.h>

#include "Platform/Types.h"
#include "Foundation/File/Path.h"


namespace Nocturnal
{

#if wxUSE_DIRDLG || wxUSE_FILEDLG

    // The global fileicons table
    APPLICATION_API class FileIconsTable& GlobalFileIconsTable();

    class APPLICATION_API FileIconsTable //: public wxFileIconsTable
    {
    public:
        enum iconId_Type
        {
            //folder,
            //folder_open,
            //computer,
            //drive,
            //cdrom,
            //floppy,
            //removeable,
            //file,
            //executable,
            FIRST_ID = wxFileIconsTable::executable,
        };

        FileIconsTable();
        virtual ~FileIconsTable();

        int GetIconID( const wxString& extension, const wxString& mime = wxEmptyString );
        
        // which is wxIMAGE_LIST_NORMAL, wxIMAGE_LIST_SMALL, or wxIMAGE_LIST_STATE
        wxImageList* GetImageList( int which = wxIMAGE_LIST_SMALL );
        wxImageList* GetSmallImageList() { return GetImageList( wxIMAGE_LIST_SMALL ); }

    private:
        // delayed initialization - wait until first use after wxArtProvider created
        void Create();

    private:
        wxHashTable* m_HashTable;
        wxImageList* m_NormalImageList;
        wxImageList* m_SmallImageList;
        wxImageList* m_StateImageList;
    };

#endif // wxUSE_DIRDLG || wxUSE_FILEDLG

}
