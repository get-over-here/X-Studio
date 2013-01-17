//
// Main Window (Menu).cpp : Handlers for the various menu/toolbar commands 
//
// NB: Best viewed with tab size of 3 characters and Visual Studio's 'XML Doc Comment' syntax colouring
//         set to a colour that highly contrasts the 'C/C++ comment' syntax colouring
// 

#include "stdafx.h"

/// ////////////////////////////////////////////////////////////////////////////////////////////////////
///                                          CONSTANTS
/// ////////////////////////////////////////////////////////////////////////////////////////////////////

// Template used to create new MSCI scripts
//
CONST TCHAR*  szScriptTemplate = TEXT("*************************************************\r\n")
                                 TEXT("* SCRIPT NAME: %s\r\n") 
                                 TEXT("* DESCRIPTION: \r\n") 
                                 TEXT("* \r\n") 
                                 TEXT("* AUTHOR: %s          DATE: %s\r\n") 
                                 TEXT("*************************************************\r\n")
                                 TEXT("\r\n") 
                                 TEXT("return null");

/// ////////////////////////////////////////////////////////////////////////////////////////////////////
///                                          FUNCTIONS
/// ////////////////////////////////////////////////////////////////////////////////////////////////////

/// ////////////////////////////////////////////////////////////////////////////////////////////////////
///                                          MESSAGE HANDLERS
/// ////////////////////////////////////////////////////////////////////////////////////////////////////

/// Function name  : onMainWindowCommand
// Description     : Menu command routing
// 
// MAIN_WINDOW_DATA*  pWindowData : [in] Main window data
// CONST UINT         iCommandID  : [in] Menu ID of the item chosen
// 
// Return type : TRUE if processed, FALSE to pass to default window proc
//
BOOL  onMainWindowCommand(MAIN_WINDOW_DATA*  pWindowData, CONST UINT  iCommandID)
{
   DOCUMENT*  pActiveDocument;
   BOOL       bResult;

   // Prepare
   TRACK_FUNCTION();
   bResult = TRUE;

   // Examine command
   switch (iCommandID)
   {
   /// [ACCELERATORS]
   case IDM_CODE_EDIT_VIEW_SUGGESTIONS:
   //case IDM_CODE_EDIT_LOOKUP_COMMAND:
      if (pActiveDocument = getActiveDocument())
         SendMessage(pActiveDocument->hWnd, WM_COMMAND, iCommandID, NULL);
      else
         bResult = FALSE;
      break;

   /// [FILE MENU]
   // [FILE: CLOSE DOCUMENT]
   case IDM_FILE_CLOSE:
      onMainWindowFileClose(pWindowData);
      break;

   // [FILE: CLOSE PROJECT]
   case IDM_FILE_CLOSE_PROJECT:
      onMainWindowFileCloseProject(pWindowData);
      break;

   // [FILE: NEW DOCUMENT]
   case IDM_FILE_NEW:
      onMainWindowFileNewDocument(pWindowData);
      break;

   // [FILE: NEW LANGUAGE FILE]
   case IDM_FILE_NEW_LANGUAGE:
      onMainWindowFileNewLanguageDocument(pWindowData, NULL);
      break;

   // [FILE: NEW SCRIPT FILE]
   case IDM_FILE_NEW_SCRIPT:
      onMainWindowFileNewScriptDocument(pWindowData, NULL);
      break;

   // [FILE: NEW PROJECT FILE]
   case IDM_FILE_NEW_PROJECT:
      onMainWindowFileNewProjectDocument(pWindowData, NULL);
      break;

   // [FILE: OPEN FILE]
   case IDM_FILE_OPEN:
      onMainWindowFileOpen(pWindowData, TRUE);
      break;

   // [FILE: SCRIPT BROWSER]
   case IDM_FILE_BROWSE:
      onMainWindowFileOpen(pWindowData, FALSE);
      break;

   // [FILE: SAVE DOCUMENT]
   case IDM_FILE_SAVE:
      onMainWindowFileSave(pWindowData);
      break;

   // [FILE: EXPORT PROJECT]
   case IDM_FILE_EXPORT_PROJECT:
      onMainWindowFileExportProject(pWindowData);
      break;

   // [FILE: SAVE DOCUMENT AS]
   case IDM_FILE_SAVE_AS:
      onMainWindowFileSaveAs(pWindowData);
      break;

   // [FILE: SAVE DOCUMENT COPY]
   case IDM_FILE_SAVE_ALL:
      onMainWindowFileSaveAll(pWindowData);
      break;

   // [FILE: EXIT]
   case IDM_FILE_EXIT:
      onMainWindowFileExit(pWindowData);
      break;

   /// [EDIT MENU]
   // [EDIT: CUT/COPY/PASTE/DELETE]
   case IDM_EDIT_CUT:         SendMessage(GetFocus(), WM_CUT,   NULL, NULL);    break;
   case IDM_EDIT_COPY:        SendMessage(GetFocus(), WM_COPY,  NULL, NULL);    break;
   case IDM_EDIT_PASTE:       SendMessage(GetFocus(), WM_PASTE, NULL, NULL);    break;
   case IDM_EDIT_DELETE:      SendMessage(GetFocus(), WM_CLEAR, NULL, NULL);    break;
   case IDM_EDIT_SELECT_ALL:  SendMessage(GetFocus(), EM_SETSEL, 0, -1);        break;
   case IDM_EDIT_UNDO:        SendMessage(GetFocus(), WM_UNDO, NULL, NULL);     break;
   case IDM_EDIT_REDO:        SendMessage(GetFocus(), EM_REDO, NULL, NULL);     break;

   // [EDIT: COMMENT] Pass to document   
   case IDM_EDIT_COMMENT:
      if (pActiveDocument = getActiveScriptDocument())
         SendMessage(pActiveDocument->hWnd, WM_COMMAND, iCommandID, NULL);
      break;

   // [EDIT: FIND] Ensure document exists
   case IDM_EDIT_FIND:
      if (pActiveDocument = getActiveDocument())
         onMainWindowEditFindText(pWindowData);
      break;

   /// [VIEW MENU]
   // [VIEW: COMMAND LIST]
   case IDM_VIEW_COMMAND_LIST:
      onMainWindowViewSearchDialog(pWindowData, RT_COMMANDS);
      break;

   // [VIEW: GAME OBJECTS LIST]
   case IDM_VIEW_GAME_OBJECTS_LIST:
      onMainWindowViewSearchDialog(pWindowData, RT_GAME_OBJECTS);
      break;

   // [VIEW: SCRIPT OBJECTS LIST]
   case IDM_VIEW_SCRIPT_OBJECTS_LIST:
      onMainWindowViewSearchDialog(pWindowData, RT_SCRIPT_OBJECTS);
      break;

   // [VIEW: OUTPUT WINDOW]
   case IDM_VIEW_COMPILER_OUTPUT:
      onMainWindowViewOutputDialog(pWindowData);
      break;

   // [VIEW: PROJECT WINDOW]
   case IDM_VIEW_PROJECT_EXPLORER:
      onMainWindowViewProjectDialog(pWindowData);
      break;

   // [VIEW: SCRIPT PROPERTIES]
   case IDM_VIEW_DOCUMENT_PROPERTIES:
      onMainWindowViewPropertiesDialog(pWindowData);
      break;

   // [VIEW: APP PREFERENCES]
   case IDM_VIEW_PREFERENCES:
      onMainWindowViewPreferencesDialog(pWindowData);
      break;

   /// [DATA MENU]
   // [DATA: GAME STRINGS]
   case IDM_TOOLS_GAME_STRINGS:
      onMainWindowDataGameStrings(pWindowData);
      break;

   // [DATA: GAME MEDIA]
   case IDM_TOOLS_MEDIA_BROWSER:
      onMainWindowDataMediaBrowser(pWindowData);
      break;

   // [DATA: MISSIONS]
   case IDM_TOOLS_MISSION_HIERARCHY:
      onMainWindowDataMissionHierarchy(pWindowData);
      break;

   // [DATA: CONVERSATIONS]
   case IDM_TOOLS_CONVERSATION_BROWSER:
      onMainWindowDataConversationBrowser(pWindowData);
      break;

   // [DATA: RELOAD]
   case IDM_TOOLS_RELOAD_GAME_DATA:
      onMainWindowDataReload(pWindowData);
      break;

   /// [WINDOW MENU]
   // [WINDOW: ACTIVATE DOCUMENT]
   default:
      // [DOCUMENT] Display document
      if (iCommandID >= IDM_WINDOW_FIRST_DOCUMENT AND iCommandID <= IDM_WINDOW_LAST_DOCUMENT)
         onMainWindowWindowShowDocument(pWindowData, iCommandID - IDM_WINDOW_FIRST_DOCUMENT);
      // [RECENT FILE] Open file
      else if (iCommandID >= IDM_FILE_RECENT_FIRST AND iCommandID <= IDM_FILE_RECENT_LAST)
         onMainWindowFileOpenRecent(pWindowData, iCommandID - IDM_FILE_RECENT_FIRST);
      // [UNHANDLED] Return FALSE
      else
         bResult = FALSE;
      break;

   // [WINDOW: CLOSE ALL DOCUMENTS]
   case IDM_WINDOW_CLOSE_ALL_DOCUMENTS:
      onMainWindowWindowCloseDocuments(pWindowData, FALSE);
      break;

   // [WINDOW: CLOSE OTHER DOCUMENTS]
   case IDM_WINDOW_CLOSE_OTHER_DOCUMENTS:
      onMainWindowWindowCloseDocuments(pWindowData, TRUE);
      break;

   // [WINDOW: NEXT DOCUMENT]
   case IDM_WINDOW_NEXT_DOCUMENT:
      displayNextDocument(pWindowData->hDocumentsTab);
      break;

   /// [HELP MENU]
   // [HELP: HELP FILE]
   case IDM_HELP_HELP:
      onMainWindowHelpFile(pWindowData);
      break;

   // [HELP: CHECK UPDATES]
   case IDM_HELP_UPDATES:
      onMainWindowHelpUpdates(pWindowData);
      break;

   // [HELP: FORUMS]
   case IDM_HELP_FORUMS:
      onMainWindowHelpForums(pWindowData);
      break;

   // [HELP: SUBMIT FILE]
   case IDM_HELP_SUBMIT_FILE:
      onMainWindowHelpSubmitFile(pWindowData);
      break;

   // [HELP: ABOUT BOX]
   case IDM_HELP_ABOUT:
      onMainWindowHelpAbout(pWindowData);
      break;

   // [HELP: TUTORIAL]
   case IDM_HELP_TUTORIAL_OPEN_FILE:    case IDM_HELP_TUTORIAL_FILE_OPTIONS:  case IDM_HELP_TUTORIAL_GAME_DATA:
   case IDM_HELP_TUTORIAL_GAME_FOLDER:  case IDM_HELP_TUTORIAL_GAME_OBJECTS:  case IDM_HELP_TUTORIAL_EDITOR:
   case IDM_HELP_TUTORIAL_PROJECTS:     case IDM_HELP_TUTORIAL_COMMANDS:      case IDM_HELP_TUTORIAL_SCRIPT_OBJECTS:
      onMainWindowHelpTutorial(pWindowData, iCommandID);
      break;

   /// [TEST MENU]
   // [TEST: LOAD COMMAND DESCRIPTIONS]
   case IDM_TEST_COMMAND_DESCRIPTIONS:
      onMainWindowTestCommandDescriptions(pWindowData);
      break;

   // [TEST: GAME DATA] Run GameData TestCases
   case IDM_TEST_GAME_DATA:
      onMainWindowTestGameData(pWindowData);
      break;

   // [TEST: SCRIPT TRANSLATION] Run ScriptTranslation TestCases
   case IDM_TEST_SCRIPT_TRANSLATION:
      onMainWindowTestScriptTranslation(pWindowData);
      break;

   // [TEST: INTERPRET DOCUMENT] Attempt to compile current script
   case IDM_TEST_INTERPRET_DOCUMENT:
      onMainWindowTestScriptGeneration(pWindowData);
      break;

   // [TEST: OPEN EXAMPLE SCRIPT]
   case IDM_TEST_ORIGINAL_SCRIPT:
      onMainWindowTestOriginalScript(pWindowData);
      break;

   // [TEST: OPEN PREVIOUS OUTPUT SCRIPT]
   case IDM_TEST_OUTPUT_SCRIPT:
      onMainWindowTestOutputScript(pWindowData);
      break;

   // [TEST: TEST CURRENT CODE] Display ALL loaded wares
   case IDM_TEST_CURRENT_CODE:
      onMainWindowTestCurrentCode(pWindowData);
      break;

   // [TEST: TEST WARES] Print GameObjectNames to the console
   case IDM_TEST_VALIDATE_XML_SCRIPTS:
      onMainWindowTestValidateUserScripts(pWindowData);
      break;

   // [TEST: CUSTOM TEST CASES] Run whatever TestCase you're debugging right now
   case IDM_TEST_VALIDATE_ALL_SCRIPTS:
      onMainWindowTestValidateAllScripts(pWindowData);
      break;
   }

   // Return result
   END_TRACKING();
   return bResult;
}


/// Function name  : onMainWindowDataGameStrings
// Description     : Creates a new LanguageDocument to display the game strings
// 
// MAIN_WINDOW_DATA*  pWindowData   : [in] Main window data
// 
VOID   onMainWindowDataGameStrings(MAIN_WINDOW_DATA*  pWindowData)
{
   // [TRACK]
   TRACK_FUNCTION();
   VERBOSE_UI_COMMAND();

   // Create new document, pass NULL instead of a LanguageFile.
   onMainWindowFileNewLanguageDocument(pWindowData, NULL);

   // [TRACK]
   END_TRACKING();
}


/// Function name  : onMainWindowDataMediaBrowser
// Description     : Creates a MediaDocument to display the game media
// 
// MAIN_WINDOW_DATA*  pWindowData   : [in] Main window data
// 
VOID  onMainWindowDataMediaBrowser(MAIN_WINDOW_DATA*  pWindowData)
{
   // [TRACK]
   TRACK_FUNCTION();
   VERBOSE_UI_COMMAND();

   // Create new document and update document properties
   TODO("Re-write Media document code");
   ///addMediaDocumentToDocumentsCtrl(pWindowData->hDocumentsTab);

   // [TRACK]
   END_TRACKING();
}


/// Function name  : onMainWindowDataMissionHierarchy
// Description     : 
// 
// MAIN_WINDOW_DATA*  pWindowData : [in] Window data
//
VOID  onMainWindowDataMissionHierarchy(MAIN_WINDOW_DATA*  pWindowData)
{
   // [TRACK]
   TRACK_FUNCTION();
   VERBOSE_UI_COMMAND();

   // [ERROR] "The feature '%s' has not been implemented"
   displayMessageDialogf(NULL, IDS_FEATURE_NOT_IMPLEMENTED, MAKEINTRESOURCE(IDS_TITLE_NOT_IMPLEMENTED), MDF_OK WITH MDF_ERROR, TEXT("Mission Director Support"));

   // [TRACK]
   END_TRACKING();
}


/// Function name  : onMainWindowDataConversationBrowser
// Description     : 
// 
// MAIN_WINDOW_DATA*  pWindowData : [in] Window data
// 
VOID  onMainWindowDataConversationBrowser(MAIN_WINDOW_DATA*  pWindowData)
{
   // [TRACK]
   TRACK_FUNCTION();
   VERBOSE_UI_COMMAND();

   // [ERROR] "The feature '%s' has not been implemented"
   displayMessageDialogf(NULL, IDS_FEATURE_NOT_IMPLEMENTED, MAKEINTRESOURCE(IDS_TITLE_NOT_IMPLEMENTED), MDF_OK WITH MDF_ERROR, TEXT("Conversation Browser"));

   // [TRACK]
   END_TRACKING();
}


/// Function name  : onMainWindowDataReload
// Description     : Reloads the game data
// 
// MAIN_WINDOW_DATA*  pWindowData : [in] Window data
// 
VOID  onMainWindowDataReload(MAIN_WINDOW_DATA*  pWindowData)
{
   // [TRACK]
   TRACK_FUNCTION();
   VERBOSE_UI_COMMAND();

   // [CHECK] Are there any MODIFIED documents open?
   if (isAnyDocumentModified(pWindowData->hDocumentsTab))
   {
      /// [MODIFIED] "You must save all documents before attempting to reload your game data, would you like to save them now?"
      if (displayMessageDialogf(NULL, IDS_GENERAL_RELOAD_DOCUMENTS_OPEN, TEXT("Unsaved Documents Are Open"), MDF_YESNO WITH MDF_WARNING) == IDYES)
         // [YES] Save all documents
         onMainWindowFileSaveAll(pWindowData);
   }
   else
      /// [UNMODIFIED] Reload game data
      performMainWindowReInitialisation(pWindowData, NULL);
   
   // [TRACK]
   END_TRACKING();
}


/// Function name  : onMainWindowEditFindText
// Description     : Display find text dialog
// 
// MAIN_WINDOW_DATA*  pWindowData : [in] Window data
// 
VOID   onMainWindowEditFindText(MAIN_WINDOW_DATA*  pWindowData)
{
   // [CHECK] Does dialog already exist?
   if (pWindowData->hFindTextDlg)
      // [EXISTS] Focus dialog
      SetForegroundWindow(pWindowData->hFindTextDlg);
   else
      // [NEW] Create and display dialog
      pWindowData->hFindTextDlg = displayFindTextDialog(pWindowData->hMainWnd);
}


/// Function name  : onMainWindowFileClose
// Description     : Close the current document (if any)
// 
// MAIN_WINDOW_DATA*  pWindowData   : [in] Main window data
// 
VOID   onMainWindowFileClose(MAIN_WINDOW_DATA*  pWindowData)
{
   DOCUMENT*  pFocusedDocument;

   // [TRACK]
   TRACK_FUNCTION();
   VERBOSE_UI_COMMAND();

   // [CHECK] Ensure there's a focused document
   if (pFocusedDocument = getFocusedDocument(pWindowData))
   {
      // Examine type
      if (pFocusedDocument->eType == DT_PROJECT)
         /// [PROJECT] Attempt to close active project
         closeActiveProject(NULL);
      else
         /// [DOCUMENT] Attempt to close active document
         closeDocumentByIndex(pWindowData->hDocumentsTab, TabCtrl_GetCurSel(pWindowData->hDocumentsTab));
   }

   // [TRACK]
   END_TRACKING();
}


/// Function name  : onMainWindowFileCloseProject
// Description     : Close the current project (if any)
// 
// MAIN_WINDOW_DATA*  pWindowData   : [in] Main window data
// 
VOID   onMainWindowFileCloseProject(MAIN_WINDOW_DATA*  pWindowData)
{
   // [TRACK]
   TRACK_FUNCTION();
   VERBOSE_UI_COMMAND();

   /// Close/save active project
   closeActiveProject(NULL);
   
   // [TRACK]
   END_TRACKING();
}


/// Function name  : onMainWindowFileExit
// Description     : Attempts to close the program
// 
// MAIN_WINDOW_DATA*  pWindowData   : [in] Main window data
// 
VOID  onMainWindowFileExit(MAIN_WINDOW_DATA*  pWindowData)
{
   // [VERBOSE]
   VERBOSE_UI_COMMAND();

   // Invoke CLOSE
   postAppClose(MWS_CLOSING);
}


/// Function name  : onMainWindowFileExportProject
// Description     : Saves all files in the project to a specified folder
// 
// MAIN_WINDOW_DATA*  pWindowData   : [in] Main window data
// 
VOID  onMainWindowFileExportProject(MAIN_WINDOW_DATA*  pWindowData)
{
   PROJECT_DOCUMENT  *pProject;
   STORED_DOCUMENT   *pDocument;
   BROWSEINFO         oBrowseData;      // Properties for the browse window
   ITEMIDLIST        *pFolderObject;   // The IDList of the folder the user selects
   TCHAR             *szFolderPath,      // The path of the folder the user selects
                     *szDestination;

   // [TRACK]
   TRACK_FUNCTION();
   VERBOSE_UI_COMMAND();

   // [CHECK] Ensure active project eixsts
   if (pProject = getActiveProject())
   {
      // Prepare
      utilZeroObject(&oBrowseData, BROWSEINFO);
      szFolderPath = utilCreateEmptyPath();
      szDestination = utilCreateEmptyPath();

      // Setup dialog: "Select destination folder for project files" 
      oBrowseData.lpszTitle = utilLoadString(getResourceInstance(), IDS_GENERAL_PROJECT_EXPORT_BROWSE, 128);
      oBrowseData.hwndOwner = getAppWindow();
      oBrowseData.ulFlags   = BIF_RETURNONLYFSDIRS;

      /// Query user for a folder
      if (pFolderObject = SHBrowseForFolder(&oBrowseData))
      {
         // [SUCCESS] Resolve path
         SHGetPathFromIDList(pFolderObject, szFolderPath);
         PathAddBackslash(szFolderPath);
         
         // Iterate through MSCI scripts
         for (UINT iIndex = 0; findDocumentInProjectFileByIndex(pProject->pProjectFile, PF_SCRIPT, iIndex, pDocument); iIndex++)
         {
            /// Generate target path and copy file
            StringCchPrintf(szDestination, MAX_PATH, TEXT("%s%s"), szFolderPath, PathFindFileName(pDocument->szFullPath));
            CopyFile(pDocument->szFullPath, szDestination, TRUE);
         }

         // [SUCCESS] "Successfully exported %d project files to '%s'"
         PathRemoveBackslash(szFolderPath);
         displayMessageDialogf(NULL, IDS_GENERAL_PROJECT_EXPORT_SUCCESS, TEXT("Project Exported Successfully"), MDF_OK WITH MDF_INFORMATION, getProjectFileCountByFolder(pProject->pProjectFile, PF_SCRIPT), szFolderPath);

         // Cleanup
         CoTaskMemFree(pFolderObject);
      }

      // Cleanup
      utilDeleteStrings((TCHAR*&)oBrowseData.lpszTitle, szFolderPath, szDestination);
   }

   // [TRACK]
   END_TRACKING();
}


/// Function name  : onMainWindowFileNewDocument
// Description     : Display the 'New Document' dialog
// 
// MAIN_WINDOW_DATA*  pWindowData : [in] Main window data
// 
VOID  onMainWindowFileNewDocument(MAIN_WINDOW_DATA*  pWindowData)
{
   NEW_DOCUMENT_DATA*  pDialogData;

   // [TRACK]
   TRACK_FUNCTION();
   VERBOSE_UI_COMMAND();

   /// Display 'Insert Document' dialog and ask user for a filename
   if (pDialogData = displayInsertDocumentDialog(pWindowData))
   {
      // Create document of appropriate type
      switch (pDialogData->eNewDocumentType)
      {
      // [SCRIPT/LANGUAGE] Append new document
      case NDT_SCRIPT:      onMainWindowFileNewScriptDocument(pWindowData, pDialogData->szFullPath);     break;
      case NDT_LANGUAGE:    onMainWindowFileNewLanguageDocument(pWindowData, pDialogData->szFullPath);   break;

      // [PROJECT] Replace current project (if any)
      case NDT_PROJECT:     onMainWindowFileNewProjectDocument(pWindowData, pDialogData->szFullPath);    break;

      // [MISSION] Error!
      case NDT_MISSION:     ASSERT(FALSE);   break;
      }

      // Set path
      //setDocumentPath(getActiveDocument() or getLastDocument() etc., pDialogData->szFilename);

      // Cleanup
      deleteInsertDocumentDialogData(pDialogData);
   }

   // [TRACK]
   END_TRACKING();
}

/// Function name  : onMainWindowFileNewLanguageDocument
// Description     : Create an empty language document
// 
// MAIN_WINDOW_DATA*  pWindowData   : [in] 
// 
VOID  onMainWindowFileNewLanguageDocument(MAIN_WINDOW_DATA*  pWindowData, CONST TCHAR*  szFullPath)
{
   //LANGUAGE_FILE*  pLanguageFile;
   DOCUMENT*       pDocument;

   // [VERBOSE]
   VERBOSE_UI_COMMAND();

   // Create LanguageFile and LanguageDocument
   //pLanguageFile = createLanguageFile(LFT_STRINGS, szFullPath, TRUE);
   pDocument = createDocumentByType(DT_LANGUAGE, NULL);
   
   // [DISPLAY] Add document and activate
   appendDocument(pWindowData->hDocumentsTab, pDocument);
   displayDocumentByIndex(pWindowData->hDocumentsTab, getDocumentCount() - 1);

   // [MODIFY] Set modified and Untitled
   setDocumentModifiedFlag(pDocument, TRUE);
   pDocument->bUntitled = TRUE;

   // Update Toolbar
   updateMainWindowToolBar(pWindowData);
}


/// Function name  : onMainWindowFileNewProjectDocument
// Description     : Create an empty project document
// 
// MAIN_WINDOW_DATA*  pMainWindowDatda : [in] Window data
// CONST TCHAR*       szFullPath       : [in] Full path of project file
// 
VOID  onMainWindowFileNewProjectDocument(MAIN_WINDOW_DATA*  pWindowData, CONST TCHAR*  szFullPath)
{
   PROJECT_FILE*      pProjectFile;
   PROJECT_DOCUMENT*  pDocument;

   // [TRACK]
   TRACK_FUNCTION();
   VERBOSE_UI_COMMAND();

   // [CHECK] Query how user would like to close the existing project, if any
   switch (closeActiveProject(NULL))
   {
   /// [SAVE-CLOSE] Save project
   case DCT_SAVE_CLOSE:
      commandSaveDocument(pWindowData, getActiveProject(), TRUE, NULL);
      break;

   /// [NONE/DISCARDED] Create new project
   case DCT_DISCARD:
      // Create Untitled ProjectFile
      pProjectFile = createProjectFile(utilEither(szFullPath, TEXT("Untitled.xprj")));

      // Create ProjectDocument and set active
      pDocument = (PROJECT_DOCUMENT*)createDocumentByType(DT_PROJECT, pProjectFile);
      setActiveProject(pDocument);

      // [MODIFY] Set modified and Untitled
      setDocumentModifiedFlag(pDocument, TRUE);
      pDocument->bUntitled = TRUE;

      // Update Toolbar
      updateMainWindowToolBar(pWindowData);
      break;
   }

   // Cleanup
   END_TRACKING();
}


/// Function name  : onMainWindowFileNewScriptDocument
// Description     : Create an empty script document
// 
// MAIN_WINDOW_DATA*  pMainWindowDatda   : [in] 
// 
VOID  onMainWindowFileNewScriptDocument(MAIN_WINDOW_DATA*  pWindowData, CONST TCHAR*  szFullPath)
{
   SCRIPT_FILE  *pScriptFile;
   DOCUMENT     *pDocument;
   TCHAR        *szInitialText,
                *szDate;
   
   // [TRACK]
   TRACK_FUNCTION();
   VERBOSE_UI_COMMAND();
   
   // Prepare
   GetDateFormat(LOCALE_USER_DEFAULT, NULL, NULL, TEXT("d MMMM yyyy"), szDate = utilCreateEmptyString(32), 32);

   // Generate empty script text
   szInitialText = utilCreateStringf(512, szScriptTemplate, szFullPath ? PathFindFileName(szFullPath) : TEXT("Untitled"), getAppPreferences()->szForumUserName, szDate);

   /// Create Untitled ScriptFile
   pScriptFile = createScriptFileFromText(szInitialText, utilEither(szFullPath, TEXT("Untitled.xml")));

   /// Create Script document
   pDocument = createDocumentByType(DT_SCRIPT, pScriptFile);
         
   // [DISPLAY] Add document and activate
   appendDocument(pWindowData->hDocumentsTab, pDocument);
   displayDocumentByIndex(pWindowData->hDocumentsTab, getDocumentCount() - 1);

   // [MODIFY]
   setDocumentModifiedFlag(pDocument, TRUE);
   pDocument->bUntitled = TRUE;

   // Cleanup and update toolbar
   updateMainWindowToolBar(pWindowData);
   utilDeleteStrings(szInitialText, szDate);
   END_TRACKING();
}


/// Function name  : onMainWindowFileOpen
// Description     : Launches the system OpenFile common dialog and opens the chosen file
//
// HWND  hMainWindow : [in] Window handle of the main BearScript window
// 
VOID   onMainWindowFileOpen(MAIN_WINDOW_DATA*  pWindowData, CONST BOOL  bUseSystemDialog)
{
   FILE_DIALOG_DATA*  pFileDialogData;
   STORED_DOCUMENT*   pStoredFile;

   // [TRACK]
   TRACK_FUNCTION();
   VERBOSE_UI_COMMAND();

   // Create 'open' dialog data
   pFileDialogData = createFileDialogData(FDT_OPEN, getAppPreferences()->szLastFolder, NULL);

   /// Display system/browser dialog
   if (displayFileDialog(pFileDialogData, pWindowData->hMainWnd, bUseSystemDialog) == IDOK)
   {
      // Iterate through returned filenames
      for (UINT  iIndex = 0; findFileDialogOutputFileByIndex(pFileDialogData, iIndex, pStoredFile); iIndex++)
         /// Load file. Activate the final document.
         commandLoadDocument(pWindowData, pStoredFile->eType, pStoredFile->szFullPath, (iIndex == pFileDialogData->iOutputFiles - 1), &pFileDialogData->oAdvanced);
   }

   // Cleanup
   deleteFileDialogData(pFileDialogData);
   END_TRACKING();
}


/// Function name  : onMainWindowFileOpenRecent
// Description     : Opens a file from the 'recent documents' menu
// 
// MAIN_WINDOW_DATA*  pWindowData : [in] Window data
// CONST UINT         iIndex      : [in] Zero-based index of the recent file to open
// 
VOID  onMainWindowFileOpenRecent(MAIN_WINDOW_DATA*  pWindowData, CONST UINT  iIndex)
{
   STORED_DOCUMENT*  pRecentFile;
   LIST*             pRecentFileList;

   // [TRACK]
   TRACK_FUNCTION();
   VERBOSE_UI_COMMAND();

   // Prepare
   pRecentFileList = generateRecentDocumentList(pWindowData);

   // [CHECK] Lookup recent file
   if (findListObjectByIndex(pRecentFileList, iIndex, (LPARAM&)pRecentFile))
      // [FOUND] Attempt to open file
      commandLoadDocument(pWindowData, pRecentFile->eType, pRecentFile->szFullPath, TRUE, NULL);

   // Cleanup
   deleteList(pRecentFileList);
   END_TRACKING();
}


/// Function name  : onMainWindowFileSave
// Description     : Save the active document or project
// 
// MAIN_WINDOW_DATA*  pWindowData : [in] Main window data
// 
VOID  onMainWindowFileSave(MAIN_WINDOW_DATA*  pWindowData)
{
   DOCUMENT*   pFocusedDocument;      // Focused document or project

   // [TRACK]
   TRACK_FUNCTION();
   VERBOSE_UI_COMMAND();

   /// Save focused document
   if (pFocusedDocument = getFocusedDocument(pWindowData))
      saveDocument(pFocusedDocument, FALSE, FALSE);
   
   // [TRACK]
   END_TRACKING();
}


/// Function name  : onMainWindowFileSaveAs
// Description     : Launches the 'Save As' dialog and saves the active document or project under a new path
// 
// MAIN_WINDOW_DATA*  pWindowData : [in] Main window data
// 
VOID  onMainWindowFileSaveAs(MAIN_WINDOW_DATA*  pWindowData)
{
   DOCUMENT*    pFocusedDocument;    // Focused document or project

   // [TRACK]
   TRACK_FUNCTION();
   VERBOSE_UI_COMMAND();

   /// Display SaveAs dialog and save document/project under new filename
   if (pFocusedDocument = getFocusedDocument(pWindowData))
      saveDocument(pFocusedDocument, TRUE, FALSE);

   // [TRACK]
   END_TRACKING();
}


/// Function name  : onMainWindowFileSaveAll
// Description     : Saves all open documents and the active project
// 
// MAIN_WINDOW_DATA*  pWindowData : [in] Main window data
// 
VOID  onMainWindowFileSaveAll(MAIN_WINDOW_DATA*  pWindowData)
{
   DOCUMENT*   pDocument;     // Document currently being processed

   // [TRACK]
   TRACK_FUNCTION();
   VERBOSE_UI_COMMAND();

   // Iterate through all open documents
   for (UINT  iIndex = 0; findDocumentByIndex(pWindowData->hDocumentsTab, iIndex, pDocument); iIndex++)
      /// [DOCUMENT] Save document
      saveDocument(pDocument, FALSE, FALSE);

   /// [PROJECT] Save project
   if (pDocument = getActiveProject())
      saveDocument(pDocument, FALSE, FALSE);

   // [TRACK]
   END_TRACKING();
}


/// Function name  : onMainWindowHelpAbout
// Description     : Display the about box
// 
// MAIN_WINDOW_DATA*  pWindowData : [in] Main window data
// 
VOID   onMainWindowHelpAbout(MAIN_WINDOW_DATA*  pWindowData)
{
   // [VERBOSE]
   VERBOSE_UI_COMMAND();

   // Launch the About Dialog
   DialogBox(getResourceInstance(), TEXT("ABOUT_DIALOG"), pWindowData->hMainWnd, dlgprocAboutBox);
   
}


/// Function name  : onMainWindowHelpFile
// Description     : 
// 
// MAIN_WINDOW_DATA*  pWindowData : [in] Main window data
// 
VOID  onMainWindowHelpFile(MAIN_WINDOW_DATA*  pWindowData)
{
   // [TRACK]
   TRACK_FUNCTION();
   VERBOSE_UI_COMMAND();

   // Display Help
   displayHelp(TEXT("XStudio_Welcome"));

   // [TRACK]
   END_TRACKING();
}


/// Function name  : onMainWindowHelpForums
// Description     : Launch the forums using the default web browser
// 
// MAIN_WINDOW_DATA*  pWindowData   : [in] Main window data
// 
VOID   onMainWindowHelpForums(MAIN_WINDOW_DATA*  pWindowData)
{
   // [TRACK]
   TRACK_FUNCTION();
   VERBOSE_UI_COMMAND();

   // Launch forum URL
   utilLaunchURL(pWindowData->hMainWnd, TEXT("http://forum.egosoft.com"), SW_SHOWMAXIMIZED);

   // [TRACK]
   END_TRACKING();
}


/// Function name  : onMainWindowHelpSubmitFile
// Description     : Create a new email with the appropriate address and subject
// 
// MAIN_WINDOW_DATA*  pWindowData   : [in] Main window data
// 
VOID   onMainWindowHelpSubmitFile(MAIN_WINDOW_DATA*  pWindowData)
{
   // [TRACK]
   TRACK_FUNCTION();
   VERBOSE_UI_COMMAND();

   /// Display bug report dialog
   if (displayBugReportDialog(pWindowData->hMainWnd, TRUE) == IDOK)
      // [SUBMIT] Launch the submission thread
      commandSubmitReport(pWindowData, NULL);

   // [TRACK]
   END_TRACKING();
}


/// Function name  : onMainWindowHelpTutorial
// Description     : Manually display any of the tutorials
// 
// MAIN_WINDOW_DATA*  pWindowData : [in] Window data
// CONST UINT         iCommandID  : [in] ID of tutorial menu command
// 
VOID  onMainWindowHelpTutorial(MAIN_WINDOW_DATA*  pWindowData, CONST UINT  iCommandID)
{
   TUTORIAL_WINDOW  eTutorial;

   // [TRACK]
   TRACK_FUNCTION();
   VERBOSE_UI_COMMAND();

   // Examine command ID
   switch (iCommandID)
   {
   case IDM_HELP_TUTORIAL_OPEN_FILE:         eTutorial = TW_OPEN_FILE;          break;
   case IDM_HELP_TUTORIAL_FILE_OPTIONS:      eTutorial = TW_FILE_OPTIONS;       break;  
   case IDM_HELP_TUTORIAL_GAME_DATA:         eTutorial = TW_GAME_DATA;          break;
   case IDM_HELP_TUTORIAL_GAME_FOLDER:       eTutorial = TW_GAME_FOLDER;        break; 
   case IDM_HELP_TUTORIAL_GAME_OBJECTS:      eTutorial = TW_GAME_OBJECTS;       break;  
   case IDM_HELP_TUTORIAL_EDITOR:            eTutorial = TW_SCRIPT_EDITING;     break;
   case IDM_HELP_TUTORIAL_PROJECTS:          eTutorial = TW_PROJECTS;           break;    
   case IDM_HELP_TUTORIAL_COMMANDS:          eTutorial = TW_SCRIPT_COMMANDS;    break;  
   case IDM_HELP_TUTORIAL_SCRIPT_OBJECTS:    eTutorial = TW_SCRIPT_OBJECTS;     break;
   default:                                  return;
   }

   /// Force display of tutorial window
   displayTutorialDialog(utilGetTopWindow(getAppWindow()), eTutorial, TRUE);

   // [TRACK]
   END_TRACKING();
}

/// Function name  : onMainWindowHelpUpdates
// Description     : 
// 
// MAIN_WINDOW_DATA*  pWindowData   : [in] 
// 
VOID  onMainWindowHelpUpdates(MAIN_WINDOW_DATA*  pWindowData)
{
   // [TRACK]
   TRACK_FUNCTION();
   VERBOSE_UI_COMMAND();

   // [ERROR] "The feature '%s' has not been implemented"
   displayMessageDialogf(NULL, IDS_FEATURE_NOT_IMPLEMENTED, MAKEINTRESOURCE(IDS_TITLE_NOT_IMPLEMENTED), MDF_OK WITH MDF_ERROR, TEXT("Automatic Updates"));

   // [TRACK]
   END_TRACKING();
}
 

/// Function name  : onMainWindowViewOutputDialog
// Description     : Toggle display of the output dialog
// 
// MAIN_WINDOW_DATA*  pWindowData  : [in] Window data
// 
VOID    onMainWindowViewOutputDialog(MAIN_WINDOW_DATA*  pWindowData)
{
   // [VERBOSE]
   TRACK_FUNCTION();
   VERBOSE_UI_COMMAND();

   /// Toggle output dialog
   displayOutputDialog(pWindowData, !pWindowData->hOutputDlg);

   // [TRACK]
   END_TRACKING();
}


/// Function name  : onMainWindowViewPreferencesDialog
// Description     : Display the preferences dialog to the user and (possibly) load game data because of it.
// 
// MAIN_WINDOW_DATA*  pWindowData  : [in] Main window data
// 
VOID   onMainWindowViewPreferencesDialog(MAIN_WINDOW_DATA*  pWindowData)
{
   UINT    iPreferencesChanged;    // Flag indicating what (if any) important preferences have changed

   // [VERBOSE]
   TRACK_FUNCTION();
   VERBOSE_UI_COMMAND();

   /// Display preferences dialog
   iPreferencesChanged = displayPreferencesDialog(pWindowData->hMainWnd, PP_GENERAL);

   // [TRACK]
   END_TRACKING();
}


/// Function name  : onMainWindowViewProjectDialog
// Description     : Toggle display of the project dialog
// 
// MAIN_WINDOW_DATA*  pWindowData : [in] Main window data
// 
VOID  onMainWindowViewProjectDialog(MAIN_WINDOW_DATA*  pWindowData)
{
   // [TRACK]
   TRACK_FUNCTION();
   VERBOSE_UI_COMMAND();

   /// Toggle project dialog
   displayProjectDialog(pWindowData, !pWindowData->hProjectDlg);

   // [TRACK]
   END_TRACKING();
}


/// Function name  : onMainWindowViewPropertiesDialog
// Description     : Display or hide the document properties dialog
// 
// MAIN_WINDOW_DATA*  pWindowData  : [in] Main window data
// 
VOID   onMainWindowViewPropertiesDialog(MAIN_WINDOW_DATA*  pWindowData)
{
   // [TRACK]
   TRACK_FUNCTION();
   VERBOSE_UI_COMMAND();

   // Display/Hide properties window
   displayPropertiesDialog(pWindowData, !pWindowData->hPropertiesSheet);

   // [TRACK]
   END_TRACKING();
}


/// Function name  : onMainWindowViewSearchDialog
// Description     : Either toggles the search results dialog, or changes the tab
// 
// MAIN_WINDOW_DATA*  pWindowData : [in] Main window data
// CONST RESULT_TYPE  eDialog     : [in] Tab to display
// 
VOID    onMainWindowViewSearchDialog(MAIN_WINDOW_DATA*  pWindowData, CONST RESULT_TYPE  eDialog)
{
   // [TRACK]
   TRACK_FUNCTION();
   VERBOSE_UI_COMMAND();

   // Change to the desired tab, unless tab is already displayed -- then dialog is hidden
   displaySearchDialog(pWindowData, eDialog);

   // [TRACK]
   END_TRACKING();
}


/// Configuration  : _TESTING
/// Function name  : onMainWindowTestCaseBegin
// Description     : Initiate the specified TestCase
// 
// MAIN_WINDOW_DATA*      pWindowData : [in] Main window data
// CONST TEST_CASE_TYPE  eType           : [in] Type of TestCase to run
// CONST UINT            iTestCaseID     : [in] ID of the TestCase to run
// 
#ifdef _TESTING
VOID  onMainWindowTestCaseBegin(MAIN_WINDOW_DATA*  pWindowData, CONST TEST_CASE_TYPE  eType, CONST UINT  iTestCaseID)
{
   switch (eType)
   {
   // [GAME DATA]
   case TCT_GAME_DATA:
      runGameDataTestCase(pWindowData, iTestCaseID);
      break;

   // [SCRIPT TRANSLATION]
   case TCT_SCRIPT_TRANSLATION:
      runScriptTranslationTestCase(pWindowData, iTestCaseID);
      break;

   // [ERROR]
   default:
      ASSERT(FALSE);
   }
}
#endif

/// Function name  : onMainWindowTestCommandDescriptions
// Description     : Load the command descriptions file for editing
// 
// MAIN_WINDOW_DATA*  pWindowData   : [in] 
// 
// Return Value   : 
// 
BOOL   onMainWindowTestCommandDescriptions(MAIN_WINDOW_DATA*  pWindowData)
{
   /*LANGUAGE_FILE*  pLanguageFile;
   OPERATION_DATA*    pOperationData;

   pLanguageFile = createLanguageFile(TEXT("Command-Descriptions.XML")); 

   pOperationData = commandLoadLanguageFile(pLanguageFile);
   createProgressDialog(pWindowData, pOperationData);*/

   // Test output dialog
   //testOutputDialog(pWindowData->hOutputDlg);

   // [VERBOSE]
   VERBOSE_UI_COMMAND();

   // [ERROR] "The feature '%s' has not been implemented"
   displayMessageDialogf(NULL, IDS_FEATURE_NOT_IMPLEMENTED, MAKEINTRESOURCE(IDS_TITLE_NOT_IMPLEMENTED), MDF_OK WITH MDF_INFORMATION, TEXT("Test command Descriptions"));
   
   return TRUE;
}


/// Function name  : onMainWindowTestCurrentCode
// Description     : Test whatever you're debugging right now
// 
// MAIN_WINDOW_DATA*  pWindowData : [in] Main window data
// 
VOID  onMainWindowTestCurrentCode(MAIN_WINDOW_DATA*  pWindowData)
{
   //SCRIPT_DOCUMENT*  pDocument;

   /*if (pDocument = getActiveScriptDocument())
      CodeEdit_FindText(pDocument->hCodeEdit, CSF_FROM_CARET, TEXT("script"));*/
}


/// Function name  : onMainWindowTestGameData
// Description     : Initiate the GameData batch of test cases
// 
// MAIN_WINDOW_DATA*  pWindowData : [in] Main window data
// 
// Return Value   : TRUE
// 
BOOL   onMainWindowTestGameData(MAIN_WINDOW_DATA*  pWindowData)
{
   // Initiate GameData TestCase number 1
   PostMessage(pWindowData->hMainWnd, UM_TESTCASE_BEGIN, TCT_GAME_DATA, 1);

   return TRUE;
}


/// Function name  : onMainWindowTestScriptGeneration
// Description     : Initiate the ScriptTranslation batch of test cases
// 
// MAIN_WINDOW_DATA*  pWindowData : [in] Main window data
// 
// Return Value   : TRUE
// 
BOOL  onMainWindowTestScriptGeneration(MAIN_WINDOW_DATA*  pWindowData)
{
   SCRIPT_DOCUMENT*  pDocument;
   ERROR_QUEUE*      pErrorQueue;

   // Prepare
   pErrorQueue = createErrorQueue();
 
   // Get active document
   if (getActiveDocument() AND getActiveDocument()->eType == DT_SCRIPT)
   {
      pDocument = (SCRIPT_DOCUMENT*)getActiveDocument();
      generateScript(pDocument->hCodeEdit, pDocument->pScriptFile, NULL, pErrorQueue);
   }

   // Cleanup and return
   deleteErrorQueue(pErrorQueue);
   return TRUE;
}


/// Function name  : onMainWindowTestScriptTranslation
// Description     : Initiate the ScriptTranslation batch of test cases
// 
// MAIN_WINDOW_DATA*  pWindowData : [in] Main window data
// 
// Return Value   : TRUE
// 
BOOL  onMainWindowTestScriptTranslation(MAIN_WINDOW_DATA*  pWindowData)
{
   // Initiate GameData TestCase number 50
   PostMessage(pWindowData->hMainWnd, UM_TESTCASE_BEGIN, TCT_SCRIPT_TRANSLATION, TC_MISSING_SCRIPT_TAG);
   return TRUE;
}


/// Function name  : onMainWindowTestOriginalScript
// Description     : Load an original script to verify the translation process
// 
// MAIN_WINDOW_DATA*  pWindowData : [in] Main window data
// 
// Return Value   : TRUE
// 
BOOL   onMainWindowTestOriginalScript(MAIN_WINDOW_DATA*  pWindowData)
{
   commandLoadDocument(pWindowData, FIF_SCRIPT, TEXT("C:\\temp\\plugin.piracy.lib.logic.xml"), TRUE, NULL);
   return TRUE;
}

/// Function name  : onMainWindowTestOutputScript
// Description     : Load a previously compiled script to verify the compiling process
// 
// MAIN_WINDOW_DATA*  pWindowData : [in] Main window data
// 
// Return Value   : TRUE
// 
BOOL   onMainWindowTestOutputScript(MAIN_WINDOW_DATA*  pWindowData)
{
   commandLoadDocument(pWindowData, FIF_SCRIPT, TEXT("C:\\Output.Test.xml"), TRUE, NULL);
   return TRUE;
}


/// Function name  : onMainWindowTestValidateAllScripts
// Description     : Validates all scripts in the scripts folder
// 
// MAIN_WINDOW_DATA*  pWindowData : [in] Main window data
// 
VOID  onMainWindowTestValidateAllScripts(MAIN_WINDOW_DATA*  pWindowData)
{
   performScriptValidationBatchTest(pWindowData, FF_SCRIPT_FILES);
}


/// Function name  : onMainWindowTestValidateUserScripts
// Description     : Validates all XML files in the scripts folder
// 
// MAIN_WINDOW_DATA*  pWindowData : [in] Main window data
// 
VOID   onMainWindowTestValidateUserScripts(MAIN_WINDOW_DATA*  pWindowData)
{
   performScriptValidationBatchTest(pWindowData, FF_SCRIPT_XML_FILES);
}



/// Function name  : onMainWindowWindowCloseDocuments
// Description     : Close all documents
// 
// MAIN_WINDOW_DATA*  pWindowData : [in] Main window data
// CONST BOOL         bExcludeActive  : [in] Whether to leave the active document open
// 
VOID  onMainWindowWindowCloseDocuments(MAIN_WINDOW_DATA*  pWindowData, CONST BOOL  bExcludeActive)
{
   // [TRACK]
   TRACK_FUNCTION();
   VERBOSE_UI_COMMAND();
   
   /// Remove all documents [except active]
   closeAllDocuments(pWindowData->hDocumentsTab, bExcludeActive);

   // [TRACK]
   END_TRACKING();
}


/// Function name  : onMainWindowWindowShowDocument
// Description     : Switch to the specified document
// 
// MAIN_WINDOW_DATA*  pWindowData : [in] Main window data
// CONST UINT         iIndex          : [in] Zero based document index
// 
VOID  onMainWindowWindowShowDocument(MAIN_WINDOW_DATA*  pWindowData, CONST UINT  iIndex)
{
   // [TRACK]
   TRACK_FUNCTION();
   VERBOSE_UI_COMMAND();
   
   /// Display desired document
   displayDocumentByIndex(pWindowData->hDocumentsTab, iIndex);

   // [TRACK]
   END_TRACKING();
}
