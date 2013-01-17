//
// Main Window (UI).cpp : Implements the high level functionality of the Main Window
//
// NB: Best viewed with tab size of 3 characters and Visual Studio's 'XML Doc Comment' syntax colouring
//         set to a colour that highly contrasts the 'C/C++ comment' syntax colouring
// 

#include "stdafx.h"

/// /////////////////////////////////////////////////////////////////////////////////////////
///                                        MACROS
/// /////////////////////////////////////////////////////////////////////////////////////////

/// /////////////////////////////////////////////////////////////////////////////////////////
///                                    CONSTANTS / GLOBALS
/// /////////////////////////////////////////////////////////////////////////////////////////


/// /////////////////////////////////////////////////////////////////////////////////////////
///                                        HELPERS
/// /////////////////////////////////////////////////////////////////////////////////////////


/// /////////////////////////////////////////////////////////////////////////////////////////
///                                       FUNCTIONS
/// /////////////////////////////////////////////////////////////////////////////////////////

/// Function name  : addDocumentToRecentDocumentList
// Description     : Adds a document to the recent file list
// 
// MAIN_WINDOW_DATA*  pWindowData : [in] Window data
// CONST DOCUMENT*    pDocument   : [in] Document to add
// 
VOID  addDocumentToRecentDocumentList(MAIN_WINDOW_DATA*  pWindowData, CONST DOCUMENT*  pDocument)
{
   STORED_DOCUMENT*  pRecentFile;
   TCHAR*            szSubKey;
   LIST*             pRecentList;
   HKEY              hRecentKey;

   // Prepare
   TRACK_FUNCTION();
   pRecentList = generateRecentDocumentList(pWindowData);
   szSubKey    = utilCreateEmptyString(32);

   /// Delete entire key
   utilRegistryDeleteAppSubKey(getAppRegistryKey(), getAppRegistrySubKey(ARK_RECENT_FILES));

   // Generate sub-key
   StringCchPrintf(szSubKey, 32, TEXT("%s\\Recent0"), getAppRegistrySubKey(ARK_RECENT_FILES));

   // Create subkey
   if (hRecentKey = utilRegistryCreateAppSubKey(getAppRegistryKey(), szSubKey))
   {
      /// [SUCCESS] Store input document into registry
      utilRegistryWriteNumber(hRecentKey, TEXT("eType"),      calculateFileTypeFromDocumentType(pDocument->eType));
      utilRegistryWriteString(hRecentKey, TEXT("szFullPath"), pDocument->szFullPath);
      // Cleanup
      utilRegistryCloseKey(hRecentKey);
   }

   /// [DOCUMENTS] Rebuild recent documents list
   for (INT  iIndex = 0, iAdded = 1; iAdded < 10 AND findListObjectByIndex(pRecentList, iIndex, (LPARAM&)pRecentFile); iIndex++)
   {
      // [CHECK] Ensure input document isn't added twice
      if (utilCompareStringVariables(pDocument->szFullPath, pRecentFile->szFullPath))
         continue;

      // Generate sub-key
      StringCchPrintf(szSubKey, 32, TEXT("%s\\Recent%u"), getAppRegistrySubKey(ARK_RECENT_FILES), iAdded);

      // Create subkey
      if (hRecentKey = utilRegistryCreateAppSubKey(getAppRegistryKey(), szSubKey))
      {
         /// [SUCCESS] Store current document into registry
         utilRegistryWriteNumber(hRecentKey, TEXT("eType"),      pRecentFile->eType);
         utilRegistryWriteString(hRecentKey, TEXT("szFullPath"), pRecentFile->szFullPath);
         iAdded++;
         
         // Cleanup
         utilRegistryCloseKey(hRecentKey);
      }
   }

   // Cleanup
   deleteList(pRecentList);
   utilDeleteString(szSubKey);
   END_TRACKING();
}


/// Function name  : commandLoadGameData
// Description     : Launches the thread that loads the game data
//
// MAIN_WINDOW_DATA*  pWindowData : [in]           Main window data
// CONST UINT         iTestCaseID : [in][optional] TestCase number
//
// Return type : TRUE if operation started successfully, FALSE otherwise
//
BOOL  commandLoadGameData(MAIN_WINDOW_DATA*  pWindowData, CONST UINT  iTestCaseID)
{
   OPERATION_DATA   *pOperationData;      // Data for the operation
   BOOL              bResult;             // Operation Result

   // [TRACKING]
   TRACK_FUNCTION();
   VERBOSE_LIB_COMMAND();

   // Create thread data for a LOAD_GAME_DATA operation
   pOperationData = createGameDataOperationData();
   
#ifdef _TESTING
   // [TESTING] Store the TestCase number
   pThreadData->iTestCaseID = iTestCaseID;
#endif
   
   /// Attempt to launch thread
   bResult = launchOperation(pWindowData, pOperationData);

   // [TRACK]
   END_TRACKING();
   return bResult;
}


/// Function name  : commandLoadDocument
// Description     : Attempts to open any file using the method appropriate to it's file type
// 
// MAIN_WINDOW_DATA*       pWindowData  : [in] Main window data
// CONST FILE_ITEM_FLAG    eFileType    : [in] Indicates the file type. Must be: FIF_SCRIPT or FIF_LANGUAGE or FIF_MISSION or FIF_CONVERSATION or FIF_UNIVERSE
// CONST TCHAR*            szFullPath   : [in] Full path of the file
// CONST BOOL              bSetActive   : [in] Whether to activate the document once it's loaded
// CONST LOADING_OPTIONS*  pOptions     : [in] Advanced loading options
// 
// Return Value   : TRUE if the file was already open, or the appropriate loading thread was launched successfully, otherwise FALSE
// 
BOOL  commandLoadDocument(MAIN_WINDOW_DATA*  pWindowData, CONST FILE_ITEM_FLAG  eFileType, CONST TCHAR*  szFullPath, CONST BOOL  bSetActive, CONST LOADING_OPTIONS*  pOptions)
{
   DOCUMENT_OPERATION*  pOperationData;  // Thread data
   OPERATION_TYPE       eOperation;      // Required operation
   //PROJECT_DOCUMENT*    pProject;
   GAME_FILE*           pGameFile;       // New document data
   INT                  iDocumentIndex;  // Index of document if already open
   BOOL                 bResult;         // Operation result

   // [TRACKING]
   TRACK_FUNCTION();
   VERBOSE_LIB_COMMAND();

   // Prepare
   pGameFile = NULL;

   /// [CHECK] Is this file already open?
   if (bResult = findDocumentIndexByPath(pWindowData->hDocumentsTab, szFullPath, iDocumentIndex))
      // [SUCCESS] Display already open document
      displayDocumentByIndex(pWindowData->hDocumentsTab, iDocumentIndex);

   // [FAILURE] Examine file type
   else switch (eFileType)
   {
   /// [PROJECT] Launch project translation thread
   case FIF_PROJECT:
      // [CHECK] Query how user would like to close the existing project, if any
      if (closeActiveProject(szFullPath) == DCT_DISCARD)
      {
         // [NONE/CLOSED] Open new project immediately
         pGameFile  = createProjectFile(szFullPath);
         eOperation = OT_LOAD_PROJECT_FILE;
         bResult    = TRUE;
      }
      break;

   /// [SCRIPTS] Launch script translation or validation thread
   case FIF_SCRIPT:
      pGameFile  = createScriptFileByOperation(SFO_TRANSLATION, szFullPath);
      eOperation = (pOptions AND pOptions->eCompilerTest == ODT_VALIDATION ? OT_VALIDATE_SCRIPT_FILE : OT_LOAD_SCRIPT_FILE);
      bResult    = TRUE;
      break;

   /// [LANGUAGE FILE] Launch language translation thread
   case FIF_LANGUAGE:
      // [ERROR] "The feature '%s' has been temporarily disabled in this release"
      //displayMessageDialogf(NULL, IDS_FEATURE_DISABLED, TEXT("Feature Disabled"), MDF_OK WITH MDF_ERROR, TEXT("Language File Editing"));

      pGameFile  = createLanguageFile(LFT_STRINGS, szFullPath, TRUE);
      eOperation = OT_LOAD_LANGUAGE_FILE;
      bResult    = TRUE;
      break;

   /// [MISSION SCRIPTS] Not implemented
   case FIF_MISSION:
      // [ERROR] "The feature '%s' has not been implemented"
      displayMessageDialogf(NULL, IDS_FEATURE_NOT_IMPLEMENTED, MAKEINTRESOURCE(IDS_TITLE_NOT_IMPLEMENTED), MDF_OK WITH MDF_ERROR, TEXT("Mission Director Script Editing"));
      break;

   /// [CONVERSATIONS] Not implemented
   case FIF_CONVERSATION:
      // [ERROR] "The feature '%s' has not been implemented"
      displayMessageDialogf(NULL, IDS_FEATURE_NOT_IMPLEMENTED, MAKEINTRESOURCE(IDS_TITLE_NOT_IMPLEMENTED), MDF_OK WITH MDF_ERROR, TEXT("Conversation Browser"));
      break;

   /// [GALAXY MAPS] Not supported
   case FIF_UNIVERSE:
      // [ERROR] "Galaxy maps cannot be viewed or edited using X-Studio"
      displayMessageDialogf(NULL, IDS_GENERAL_GALAXY_MAPS_UNSUPPORTED, MAKEINTRESOURCE(IDS_TITLE_NOT_IMPLEMENTED), MDF_OK WITH MDF_ERROR);
      break;

   /// [UNSUPPORTED] Error
   default:
      // [ERROR] "Unable to identify the document type, the file may be damaged or unsupported"
      displayMessageDialogf(NULL, IDS_GENERAL_UNKNOWN_FILE_TYPE, TEXT("Unsupported Document Type"), MDF_OK WITH MDF_ERROR);
      break;
   }

   // [CHECK] Are we launching or was there an error?
   if (pGameFile)
   {
      // Create thread data
      pOperationData = createDocumentOperationData(eOperation, pGameFile);

      // Set properties
      pOperationData->bActivateOnLoad = bSetActive;

      // [CHECK] Are there advanced script loading options?
      if (eFileType == FIF_SCRIPT AND pOptions)
      {
         /// [ADVANCED] Set advanced options
         pOperationData->oAdvanced = (*pOptions);

         // [GENERATION] Store file-path for later verification
         if (pOptions->eCompilerTest == ODT_GENERATION)
            StringCchCopy(pOperationData->oAdvanced.szOriginalPath, MAX_PATH, szFullPath);
      }
      
      /// Attempt to launch thread
      bResult = launchOperation(pWindowData, pOperationData);
   }
   
   // Return result
   END_TRACKING();
   return bResult;
}


/// Function name  : commandLoadScriptDependency
// Description     : Loads the .pck version if available, otherwise the .xml version of the specified script
// 
// MAIN_WINDOW_DATA*       pWindowData : [in] Main window data
// CONST TCHAR*            szFullPath  : [in] FilePath of the file to load
///                                                      Extension should be .pck
// CONST BOOL              bSetActive  : [in] Whether to activate the document once opened
// CONST LOADING_OPTIONS*  pOptions    : [in] Advanced recursive loading options
//
// Return type : TRUE file could be found, otherwise FALSE
//
BOOL  commandLoadScriptDependency(MAIN_WINDOW_DATA*  pWindowData, CONST TCHAR*  szFullPath, CONST BOOL  bSetActive, CONST LOADING_OPTIONS*  pOptions)
{
   TCHAR*    szFilePathCopy;    // Copy of input path, used for renaming
   BOOL      bResult;           // Operation Result

   // [TRACKING]
   TRACK_FUNCTION();
   VERBOSE_LIB_COMMAND();

   // Prepare
   szFilePathCopy = utilDuplicatePath(szFullPath);
   bResult        = FALSE;

   // [CHECK] Does the PCK version exist?
   if (isFilePresent(getFileSystem(), szFilePathCopy))
      /// [SUCCESS] Attempt to open script
      bResult = commandLoadDocument(pWindowData, FIF_SCRIPT, szFilePathCopy, bSetActive, pOptions);
   else
   {
      // [FAILED] Change extension to .xml
      PathRenameExtension(szFilePathCopy, TEXT(".xml"));

      // [CHECK] Does the XML version exist?
      if (isFilePresent(getFileSystem(), szFilePathCopy))
         /// [SUCCESS] Attempt to open script
         bResult = commandLoadDocument(pWindowData, FIF_SCRIPT, szFilePathCopy, bSetActive, pOptions);
   }

   // Cleanup and return result
   utilDeleteString(szFilePathCopy);
   END_TRACKING();
   return bResult;
}


/// Function name  : commandSaveDocument
// Description     : Saves any type of document
// 
// MAIN_WINDOW_DATA*       pWindowData     : [in] Main window data
// SCRIPT_DOCUMENT*        pDocument       : [in] Document to be saved
// CONST BOOL              bCloseOnSuccess : [in] Whether to close the document after a successful save
// CONST LOADING_OPTIONS*  pOptions        : [in] Advanced saving options
// 
// Return type : TRUE if operation started successfully, FALSE otherwise
//
BOOL  commandSaveDocument(MAIN_WINDOW_DATA*  pWindowData, DOCUMENT*  pDocument, CONST BOOL  bCloseDocument, CONST LOADING_OPTIONS*  pOptions)
{
   DOCUMENT_OPERATION*  pOperationData;  // Thread data
   OPERATION_TYPE       eOperation;      // Required operation
   TCHAR*               szFolderPath;    // Folder portion of the target path
   BOOL                 bResult;         // Thread launch result

   // [TRACK]
   TRACK_FUNCTION();
   VERBOSE_LIB_COMMAND();

   // [CHECK] Document must not be virtual 
   ASSERT(!pDocument->bVirtual);

   // Prepare
   szFolderPath = utilDuplicateFolderPath(getDocumentPath(pDocument));
   bResult      = FALSE;

   // [CHECK] Ensure path exists
   if (!PathFileExists(szFolderPath))
      SHCreateDirectoryEx(getAppWindow(), szFolderPath, NULL);
   
   /// Determine appropriate save function
   switch (pDocument->eType)
   {
   case DT_LANGUAGE:  eOperation = OT_SAVE_LANGUAGE_FILE;   break;
   case DT_PROJECT:   eOperation = OT_SAVE_PROJECT_FILE;    break;
   case DT_SCRIPT:    eOperation = OT_SAVE_SCRIPT_FILE;     break;
   case DT_MEDIA:     ASSERT(FALSE); 
   }

   /// Create operation data
   pOperationData = createDocumentOperationData(eOperation, pDocument->pGameFile);

   // Set common properties
   pOperationData->pDocument = (LPARAM)pDocument;
   pDocument->bClosing       = bCloseDocument;

   // Set unique properties
   if (pDocument->eType == DT_SCRIPT)
      pOperationData->hCodeEdit = ((SCRIPT_DOCUMENT*)pDocument)->hCodeEdit;

   // [OPTIONS] Store options, if any
   if (pDocument->eType == DT_SCRIPT AND pOptions)
      pOperationData->oAdvanced = (*pOptions);

   /// Attempt to launch thread
   bResult = launchOperation(pWindowData, pOperationData);

   // Cleanup and return result
   utilDeleteString(szFolderPath);
   END_TRACKING();
   return bResult;
}


/// Function name  : commandSubmitBugReport
// Description     : Launches the bug report submission thread
//
// MAIN_WINDOW_DATA*  pWindowData   : [in] Main window data
//
// Return type : TRUE if operation started successfully, FALSE otherwise
//
BOOL  commandSubmitReport(MAIN_WINDOW_DATA*  pWindowData, CONST TCHAR*  szCorrection)
{
   SUBMISSION_OPERATION   *pOperationData;      // Data for the operation
   BOOL                    bResult;             // Operation Result

   // [TRACKING]
   TRACK_FUNCTION();
   VERBOSE_LIB_COMMAND();

   // [CHECK] Ensure forum username is present
   ASSERT(lstrlen(getAppPreferences()->szForumUserName));

   // Create thread data for a bug/correction submission operation
   pOperationData = createSubmissionOperationData(szCorrection);

   /// Attempt to launch thread
   bResult = launchOperation(pWindowData, pOperationData);

   // [TRACK]
   END_TRACKING();
   return bResult;
}


/// Function name  : displayOperationStatus
// Description     : Print the 'starting' message for an operation and assign operation a unique ID
// 
// OPERATION_DATA*   pOperation  : [in/out] Operation data
// 
VOID  displayOperationStatus(OPERATION_DATA*  pOperation)
{
   DOCUMENT_OPERATION    *pDocumentOperation;     // Convenience pointer
   SCRIPTCALL_OPERATION  *pScriptCallOperation;
   CONST TCHAR           *szGameVersion,          // Convenience string - game version
                         *szFileName;             // Convenience string - file name
   UINT                   iMessageID;             // Resource ID of the name/result
   
   // [TRACK]
   TRACK_FUNCTION();

   // Prepare
   pDocumentOperation = (DOCUMENT_OPERATION*)pOperation;
   szGameVersion      = identifyGameVersionString(getAppPreferences()->eGameVersion);
   iMessageID         = identifyOperationStatusMessageID(pOperation);

   // Format messages according to operation type
   switch (pOperation->eType)
   {
   /// [LOAD GAME DATA]
   case OT_LOAD_GAME_DATA: 
      // [OPERATION] "Loading %s game data from '%s"
      printOperationStateToOutputDialogf(pOperation, iMessageID, szGameVersion, getAppPreferences()->szGameFolder);  

      // [CHECK] Operation failed?
      if (isOperationComplete(pOperation) AND !isOperationSuccessful(pOperation))
         // [FAILURE] "Loaded %s game data successfully/failed/aborted by user"
         setMainWindowStatusBarTextf(0, iMessageID, szGameVersion);
      break;

   /// [LOAD/SAVE SCRIPT] Insert game version and filename
   case OT_LOAD_SCRIPT_FILE:
   case OT_SAVE_SCRIPT_FILE:
   case OT_VALIDATE_SCRIPT_FILE:
      // Prepare
      szFileName = PathFindFileName(pDocumentOperation->szFullPath);

      // [CHECK] Operation starting?
      if (!isOperationComplete(pOperation))
         // [OPERATION] "Loading MSCI script '%s'..."
         printOperationStateToOutputDialogf(pOperation, iMessageID, szFileName);  
      else
      {  // [RESULT] "Loaded %s script '%s' successfully/failed/aborted by user"
         printOperationStateToOutputDialogf(pOperation, iMessageID, szGameVersion, szFileName);
         setMainWindowStatusBarTextf(0, iMessageID, szGameVersion, szFileName);
      }
      break;

   /// [LOAD/SAVE DOCUMENT] Insert filename
   case OT_LOAD_LANGUAGE_FILE: 
   case OT_SAVE_LANGUAGE_FILE: 
   case OT_LOAD_PROJECT_FILE:
   case OT_SAVE_PROJECT_FILE:
      // Prepare
      szFileName = PathFindFileName(pDocumentOperation->szFullPath);

      // [OPERATION] "Loading language file '%s'..."
      printOperationStateToOutputDialogf(pOperation, iMessageID, szFileName);

      // [CHECK] Operation complete?
      if (isOperationComplete(pOperation))
         // [RESULT] "Loaded language file '%s' successfully/failed/aborted by user"
         setMainWindowStatusBarTextf(0, iMessageID, szFileName);
      break;

   /// [SCRIPT-CALLS] Add script name
   case OT_SEARCH_SCRIPT_CALLS:
      // Prepare
      pScriptCallOperation = (SCRIPTCALL_OPERATION*)pOperation;

      // [OPERATION] "Searching for scripts that depend upon '%s'..."
      printOperationStateToOutputDialogf(pOperation, iMessageID, pScriptCallOperation->szScriptName); 
      break;

   /// [SUBMIT REPORT] Output verbatim
   case OT_SUBMIT_BUG_REPORT: 
   case OT_SUBMIT_CORRECTION:
      // [OPERATION] "Submitting bug report..."
      printOperationStateToOutputDialogf(pOperation, iMessageID); 
      break;
   }

   // [TRACK]
   END_TRACKING();
}


/// Function name  : displayOutputDialog
// Description     : Shows or hides the output dialog
// 
// MAIN_WINDOW_DATA*  pWindowData : [in] Window data
// CONST BOOL         bShow       : [in] Whether to show or hide
// 
// Return Value   : TRUE if dialog was created or destroyed, otherwise FALSE
// 
BOOL    displayOutputDialog(MAIN_WINDOW_DATA*  pWindowData, CONST BOOL  bShow)
{
   PANE_PROPERTIES  oPaneData;          // Pane creation data
   PANE*            pDocumentPane;      // Workspace pane containing the documents tab and (possibly) the projects dialog
   BOOL             bResult;            // Operation result

   // [TRACK]
   TRACK_FUNCTION();
   VERBOSE_LIB_COMMAND();

   // Prepare
   bResult = FALSE;

   // [CHECK] Ensure dialog does't exist
   if (bShow AND !pWindowData->hOutputDlg)
   {
      // Lookup the workspace pane containing the DocumentsCtrl and Projects dialog
      if (findWorkspacePaneByHandle(pWindowData->hWorkspace, pWindowData->hDocumentsTab, pWindowData->hProjectDlg, NULL, pDocumentPane))
      {
         /// [CREATE] Create output dialog
         if (pWindowData->hOutputDlg = createOutputDialog(pWindowData->pOutputDlgData, pWindowData->hMainWnd))
         {
            // [CHECK] Ensure saved size is valid
            ASSERT(getAppPreferences()->iOutputDialogSplit);

            // Create an adjustable split with a small border around the dialog
            setWorkspacePaneProperties(&oPaneData, FALSE, getAppPreferences()->iOutputDialogSplit, NULL);
            SetRect(&oPaneData.rcBorder, 0, 1, 2, 0);

            /// [SUCCESS] Add output dialog to workspace
            bResult = insertWorkspaceWindow(pWindowData->hWorkspace, pDocumentPane, pWindowData->hOutputDlg, BOTTOM, &oPaneData);
         }
      }
      else
         CONSOLE_ERROR("Target pane not found");
   }
   // [CHECK] Ensure dialog exists
   else if (!bShow AND pWindowData->hOutputDlg)
   {
      // Save the window size to preferences
      setAppPreferencesDialogSplit(AW_OUTPUT, getWorkspaceWindowSize(pWindowData->hWorkspace, pWindowData->hOutputDlg));

      /// [DESTROY] Remove from workspace and destroy
      bResult = removeWorkspaceWindow(pWindowData->hWorkspace, pWindowData->hOutputDlg);
      pWindowData->hOutputDlg = NULL;
   }

   // [TOOLBAR] Update Toolbar to reflect current windows
   updateMainWindowToolBar(pWindowData);

   // Cleanup and return result
   END_TRACKING();
   return bResult;
}


/// Function name  : displayProjectDialog
// Description     : Shows or hides the project dialog
// 
// MAIN_WINDOW_DATA*  pWindowData : [in] Window data
// CONST BOOL         bShow       : [in] Whether to show or hide
// 
// Return Value   : TRUE if dialog was created or destroyed, otherwise FALSE
// 
BOOL  displayProjectDialog(MAIN_WINDOW_DATA*  pWindowData, CONST BOOL  bShow)
{
   PANE_PROPERTIES  oPaneData;          // Pane creation data
   DOCUMENT*        pProject;
   PANE*            pDocumentPane;      // Workspace pane containing the documents tab
   BOOL             bResult;            // Operation result

   // [TRACK]
   TRACK_FUNCTION();
   VERBOSE_LIB_COMMAND();

   // Prepare
   pProject = getActiveProject();
   bResult  = FALSE;

   // [CHECK] Ensure dialog doesn't exist
   if (bShow AND !pWindowData->hProjectDlg)
   {
      // Lookup the workspace pane containing the Documents Ctrl
      if (findWorkspacePaneByHandle(pWindowData->hWorkspace, pWindowData->hDocumentsTab, NULL, NULL, pDocumentPane))
      {
         /// [CREATE] Create project dialog
         if (pWindowData->hProjectDlg = createProjectDialog(pWindowData->hMainWnd))
         {
            // [CHECK] Ensure saved size is valid
            ASSERT(getAppPreferences()->iProjectDialogSplit);

            // Create an adjustable split with a small border around the dialog
            setWorkspacePaneProperties(&oPaneData, FALSE, getAppPreferences()->iProjectDialogSplit, NULL);
            SetRect(&oPaneData.rcBorder, 0, 1, 2, 0);

            /// [SUCCESS] Add project dialog to workspace
            bResult = insertWorkspaceWindow(pWindowData->hWorkspace, pDocumentPane, pWindowData->hProjectDlg, LEFT, &oPaneData);
         }
      }
      else
         CONSOLE_ERROR("Target pane not found");
   }
   // [CHECK] Ensure dialog exists
   else if (!bShow AND pWindowData->hProjectDlg)
   {
      // Save the window size to preferences
      setAppPreferencesDialogSplit(AW_PROJECT, getWorkspaceWindowSize(pWindowData->hWorkspace, pWindowData->hProjectDlg));

      /// [DESTROY] Remove from workspace and destroy
      bResult = removeWorkspaceWindow(pWindowData->hWorkspace, pWindowData->hProjectDlg);
      pWindowData->hProjectDlg = NULL;
   }

   // [PROJECT] Set/Remove document window
   if (pProject)
      pProject->hWnd = pWindowData->hProjectDlg;

   // [TUTORIAL] Delay display the search dialog tutorial
   setMainWindowTutorialTimer(pWindowData, TW_PROJECTS, TRUE);

   // [TOOLBAR] Update Toolbar to reflect current windows
   updateMainWindowToolBar(pWindowData);

   // Cleanup and return result
   END_TRACKING();
   return bResult;
}


/// Function name  : displaySearchDialog
// Description     : Displays the desired search results tab. If the tab is already active, the search dialog is destroyed
// 
// MAIN_WINDOW_DATA*  pWindowData : [in] Window data
// CONST RESULT_TYPE  eDialogType : [in] Type of dialog to create/destroy
// 
// Return Value   : TRUE if the dialog was or created/destroyed, otherwise FALSE
// 
BOOL   displaySearchDialog(MAIN_WINDOW_DATA*  pWindowData, CONST RESULT_TYPE  eDialogID)
{
   TUTORIAL_WINDOW  eTutorialWindows[3] = { TW_SCRIPT_COMMANDS, TW_GAME_OBJECTS, TW_SCRIPT_OBJECTS }; // Tutorial windows for each dialog
   PANE_PROPERTIES  oPaneData;        // Pane creation data
   PANE*            pTargetPane;      // Workspace pane containing the DocumentsCtrl, OutputDialog and ProjectDialog
   BOOL             bResult;          // Operation result
   
   // [TRACK]
   TRACK_FUNCTION();
   VERBOSE_LIB_COMMAND();

   // [CHECK] Ensure dialog ID is valid
   ASSERT(eDialogID != RT_NONE);

   // Prepare
   bResult = FALSE;

   // [CHECK] Does the SearchDialog exist?
   if (pWindowData->hSearchTabDlg)
   {
      // [CHECK] Determine whether to switch or destroy dialog
      if (eDialogID != identifyActiveResultsDialog(pWindowData->hSearchTabDlg))
         /// [SWITCH] Display desired dialog
         bResult = setActiveResultsDialogByType(pWindowData->hSearchTabDlg, eDialogID);

      else
      {
         // Save size and ID to preferences
         setAppPreferencesSearchDialogTab(eDialogID);
         setAppPreferencesDialogSplit(AW_SEARCH, getWorkspaceWindowSize(pWindowData->hWorkspace, pWindowData->hSearchTabDlg));
         // Save current filters
         setAppPreferencesSearchDialogFilter(RT_COMMANDS, identifyResultsDialogFilterByType(pWindowData->hSearchTabDlg, RT_COMMANDS));
         setAppPreferencesSearchDialogFilter(RT_GAME_OBJECTS, identifyResultsDialogFilterByType(pWindowData->hSearchTabDlg, RT_GAME_OBJECTS));
         setAppPreferencesSearchDialogFilter(RT_SCRIPT_OBJECTS, identifyResultsDialogFilterByType(pWindowData->hSearchTabDlg, RT_SCRIPT_OBJECTS));

         /// [REMOVE] Remove from workspace and destroy
         bResult = removeWorkspaceWindow(pWindowData->hWorkspace, pWindowData->hSearchTabDlg);
         pWindowData->hSearchTabDlg = NULL;
      }
   }
   // [CHECK] Lookup the workspace pane containing the DocumentsCtrl, OutputDlg and ProjectDlg
   else if (findWorkspacePaneByHandle(pWindowData->hWorkspace, pWindowData->hDocumentsTab, pWindowData->hOutputDlg, pWindowData->hProjectDlg, pTargetPane))
   {
      /// [CREATE] Create SearchDialog with the desired ResultsDialog
      if (pWindowData->hSearchTabDlg = createSearchDialog(pWindowData->hMainWnd, eDialogID))
      {
         // [CHECK] Ensure saved size is valid
         ASSERT(getAppPreferences()->iSearchDialogSplit);

         //VERBOSE("DEBUG: SAVING WORKSPACE SEARCH PANE = %d pixels", getAppPreferences()->iSearchDialogSplit);

         // Create an adjustable split and add to workspace
         setWorkspacePaneProperties(&oPaneData, FALSE, getAppPreferences()->iSearchDialogSplit, NULL);
         bResult = insertWorkspaceWindow(pWindowData->hWorkspace, pTargetPane, pWindowData->hSearchTabDlg, RIGHT, &oPaneData);
      }
   }
   else
      CONSOLE_ERROR("Target pane not found");

   // [TUTORIAL] Delay display the search dialog tutorial
   setMainWindowTutorialTimer(pWindowData, eTutorialWindows[eDialogID], TRUE);

   /// [TOOLBAR] Update Toolbar to reflect current windows
   updateMainWindowToolBar(pWindowData);

   // [TRACK] Return result
   END_TRACKING();
   return bResult;
}


/// Function name  : generateRecentDocumentList
// Description     : Loads the StoredDocuments saved in the registry, if any
// 
// MAIN_WINDOW_DATA*  pWindowData : [in] Main window data
//
// Return Value : New StoredDocument list representing each recent file
// 
LIST*  generateRecentDocumentList(MAIN_WINDOW_DATA*  pWindowData)
{
   FILE_ITEM_FLAG   eFileType;        // Recent file type
   LIST            *pOutputList;      // Recent files list
   TCHAR           *szFullPath,       // Full path to document
                   *szSubKey;         // Name of the sub-key used to access each document
   HKEY             hRecentKey;       // Sub-key used to access each document

   // Prepare
   TRACK_FUNCTION();
   szFullPath = utilCreateEmptyString(MAX_PATH);
   szSubKey   = utilCreateEmptyString(32);

   // Create output list
   pOutputList = createList(destructorStoredDocument);

   /// [DOCUMENTS] Iterate through up to 10 possible recent documents
   for (UINT  iIndex = 0; iIndex < 10; iIndex++)
   {
      // Generate document sub-key
      StringCchPrintf(szSubKey, 32, TEXT("%s\\Recent%u"), getAppRegistrySubKey(ARK_RECENT_FILES), iIndex);

      /// [CHECK] Is there another recent file?
      if (hRecentKey = utilRegistryOpenAppSubKey(getAppRegistryKey(), szSubKey))
      {
         // Read document properties from registry
         if (utilRegistryReadNumber(hRecentKey, TEXT("eType"), &eFileType) AND utilRegistryReadString(hRecentKey, TEXT("szFullPath"), szFullPath, MAX_PATH))
            /// [SUCCESS] Add to output list
            appendObjectToList(pOutputList, (LPARAM)createStoredDocument(eFileType, szFullPath));         
         
         // Cleanup
         utilRegistryCloseKey(hRecentKey);
      }
      // [FAILED] Stop searching for documents
      else
         break;
   }

   // Cleanup and return list
   utilDeleteStrings(szFullPath, szSubKey);
   END_TRACKING();
   return pOutputList;
}

/// Function name  : identifyMainWindowCommandStateByID
// Description     : Determines the state of any given toolbar/menu command
// 
// MAIN_WINDOW_DATA*  pWindowData  : [in] Main window data
// CONST UINT         iCommandID       : [in] ID of the command to query
// 
// Return Value   : Combination of MF_ENABLED, MF_DISABLED, MF_CHECKED and MF_UNCHECKED
//                   NB: MF_ENABLED and MF_UNCHECKED are NULL
// 
BOOL  identifyMainWindowCommandStateByID(MAIN_WINDOW_DATA*  pWindowData, CONST UINT  iCommandID)
{
   DOCUMENT*   pDocument;        // Used for determining whether an item in the Window menu is the active document or not
   TCHAR       szClassName[32];  // WindowClass name of the window with keyboard focus
   BOOL        iOutput;          // Operation result
   
   // Prepare
   iOutput = MF_DISABLED;

   // Examine command ID
   switch (iCommandID)
   {
   // [DO NOT REQUIRE GAME DATA] Enabled whether game data is present or not

   /// [EXIT + PREFERENCES] Always enabled
   case IDM_FILE_EXIT:
   case IDM_VIEW_PREFERENCES:
   /// [RECENT MENU] Always enabled
   case IDM_FILE_RECENT_FIRST: 
   case IDM_FILE_RECENT_FIRST+1:  case IDM_FILE_RECENT_FIRST+4:  case IDM_FILE_RECENT_FIRST+7:
   case IDM_FILE_RECENT_FIRST+2:  case IDM_FILE_RECENT_FIRST+5:  case IDM_FILE_RECENT_FIRST+8:
   case IDM_FILE_RECENT_FIRST+3:  case IDM_FILE_RECENT_FIRST+6:  case IDM_FILE_RECENT_FIRST+9:
   case IDM_FILE_RECENT_LAST:     
   /// [HELP MENU] Always enabled
   case IDM_HELP_HELP:
   case IDM_HELP_FORUMS:
   case IDM_HELP_SUBMIT_FILE:
   case IDM_HELP_TUTORIAL_OPEN_FILE:    case IDM_HELP_TUTORIAL_FILE_OPTIONS:  case IDM_HELP_TUTORIAL_GAME_DATA:
   case IDM_HELP_TUTORIAL_GAME_FOLDER:  case IDM_HELP_TUTORIAL_GAME_OBJECTS:  case IDM_HELP_TUTORIAL_EDITOR:
   case IDM_HELP_TUTORIAL_PROJECTS:     case IDM_HELP_TUTORIAL_COMMANDS:      case IDM_HELP_TUTORIAL_SCRIPT_OBJECTS:
   case IDM_HELP_ABOUT:
   /// [SYSTEM MENU] Always enabled
   case IDM_SHOW_HIDE_CONSOLE:
   case SC_CLOSE:
   case SC_MOVE:
   case SC_SIZE:
   case SC_RESTORE:
   case SC_MINIMIZE:
   case SC_MAXIMIZE:
   /// [TEST CODE] Enabled
   case IDM_TEST_CURRENT_CODE:
      iOutput = MF_ENABLED;
      break;

   /// [NOT IMPLEMENTED] Always disabled
   case IDM_FILE_NEW_LANGUAGE:
   case IDM_FILE_NEW_MISSION:
   case IDM_HELP_UPDATES:
   case IDM_TOOLS_MISSION_HIERARCHY:
   case IDM_TOOLS_CONVERSATION_BROWSER: 
   /// [RECENT PLACEHOLDER]
   case IDM_FILE_RECENT_EMPTY:
   /// [LANGUAGE/MEDIA] Temporarily disabled
   case IDM_TOOLS_GAME_STRINGS:
   case IDM_TOOLS_MEDIA_BROWSER:
   /// [OLD TEST STUFF] Always Disabled
   case IDM_TEST_OUTPUT_SCRIPT:
   case IDM_TEST_COMMAND_DESCRIPTIONS:
   case IDM_TEST_GAME_DATA:
   case IDM_TEST_SCRIPT_TRANSLATION:
   case IDM_TEST_INTERPRET_DOCUMENT:
      iOutput = MF_DISABLED;     
      break;

   /// [TOOL WINDOWS] Checked depending on visibility
   case IDM_VIEW_COMPILER_OUTPUT:             iOutput = (pWindowData->hOutputDlg ? MF_CHECKED : MF_UNCHECKED);     break;

   // [REQUIRE GAME DATA] Require game data and/or a specific state
   default:
      /// [NO GAME DATA] Disable all remaining functionality
      if (getAppState() != AS_GAME_DATA_LOADED)
         iOutput = MF_DISABLED;

      // [GAME DATA PRESENT] 
      else switch (iCommandID)
      {
      /// [VALIDATION] Enabled
      case IDM_TEST_ORIGINAL_SCRIPT:
      case IDM_TEST_VALIDATE_ALL_SCRIPTS:
      case IDM_TEST_VALIDATE_XML_SCRIPTS:
      /// [VIEW GAME DATA] Enabled
      case IDM_TOOLS_RELOAD_GAME_DATA:
      //case IDM_TOOLS_GAME_STRINGS:
      //case IDM_TOOLS_MEDIA_BROWSER:
      /// [NEW/OPEN] Enabled
      case IDM_FILE_NEW:
      case IDM_FILE_NEW_SCRIPT:
      case IDM_FILE_OPEN:       
      case IDM_FILE_BROWSE:      
         iOutput = MF_ENABLED;                                                      
         break;

      /// [TOOL WINDOWS] Checked depending on visibility
      case IDM_VIEW_COMMAND_LIST:            iOutput = (identifyActiveResultsDialog(pWindowData->hSearchTabDlg) == RT_COMMANDS       ? MF_CHECKED : MF_UNCHECKED);   break;
      case IDM_VIEW_GAME_OBJECTS_LIST:       iOutput = (identifyActiveResultsDialog(pWindowData->hSearchTabDlg) == RT_GAME_OBJECTS   ? MF_CHECKED : MF_UNCHECKED);   break;
      case IDM_VIEW_SCRIPT_OBJECTS_LIST:     iOutput = (identifyActiveResultsDialog(pWindowData->hSearchTabDlg) == RT_SCRIPT_OBJECTS ? MF_CHECKED : MF_UNCHECKED);   break;
      case IDM_VIEW_DOCUMENT_PROPERTIES:     iOutput = (pWindowData->hPropertiesSheet ? MF_CHECKED : MF_UNCHECKED);     break;   
      case IDM_VIEW_PROJECT_EXPLORER:        iOutput = (pWindowData->hProjectDlg      ? MF_CHECKED : MF_UNCHECKED);     break;

      /// [PROJECTS] Requires a project in specific state
      case IDM_FILE_NEW_PROJECT:             iOutput = (!getActiveProject()             ? MF_ENABLED : MF_DISABLED);    break;
      case IDM_FILE_EXPORT_PROJECT:          
      case IDM_FILE_CLOSE_PROJECT:           
      case IDM_PROJECT_EDIT_VARIABLES:       iOutput = (getActiveProject()              ? MF_ENABLED : MF_DISABLED);    break;

      /// [SAVE/CLOSE] Requires a document/project in specific state
      case IDM_FILE_SAVE:                    iOutput = (isModified(getFocusedDocument(pWindowData))       ? MF_ENABLED : MF_DISABLED);    break;
      case IDM_FILE_SAVE_ALL:                iOutput = (isAnyDocumentModified(pWindowData->hDocumentsTab) ? MF_ENABLED : MF_DISABLED);    break;
      case IDM_FILE_SAVE_AS:                 
      case IDM_FILE_CLOSE:                   iOutput = (getFocusedDocument(pWindowData)                   ? MF_ENABLED : MF_DISABLED);    break;

      /// [CLOSE-ALL/CLOSE-OTHER] Requires a specific number of documents
      case IDM_WINDOW_CLOSE_ALL_DOCUMENTS:   iOutput = (getActiveDocument()    ? MF_ENABLED : MF_DISABLED);    break;
      case IDM_WINDOW_CLOSE_OTHER_DOCUMENTS: iOutput = (getDocumentCount() > 1 ? MF_ENABLED : MF_DISABLED);    break;      

      /// [FIND TEXT] Requires a document
      case IDM_EDIT_FIND:                    iOutput = (getActiveDocument()    ? MF_ENABLED : MF_DISABLED);    break;

      /// [EDIT COMMANDS] Requires an Edit/CodeEdit with keyboard focus
      case IDM_EDIT_UNDO:
      case IDM_EDIT_REDO:
      case IDM_EDIT_CUT:
      case IDM_EDIT_COPY:
      case IDM_EDIT_PASTE:
      case IDM_EDIT_DELETE:
      case IDM_EDIT_SELECT_ALL:
         // [EDIT WINDOW] Always enable
         if (GetClassName(GetFocus(), szClassName, 32) AND (utilCompareStringVariables(szClassName, WC_EDIT))) // OR utilCompareStringVariables(szClassName, szCodeEditClass)))
            iOutput = MF_ENABLED;
         
         // [DOCUMENT CODE-EDIT] Query document
         else if (GetFocus() AND getActiveScriptCodeEdit() == GetFocus())
            iOutput = SendMessage(getActiveDocument()->hWnd, UM_QUERY_DOCUMENT_COMMAND, iCommandID, NULL);
         
         // [PREFERENCES CODE-EDIT] Enable all
         else if (utilCompareStringVariables(szClassName, szCodeEditClass))
            iOutput = MF_ENABLED;
         break;

      /// [COMMENT] Require CodeEdit with keyboard focus
      case IDM_EDIT_COMMENT:
         // [DOCUMENT CODE-EDIT] Query document
         if (GetFocus() AND getActiveScriptCodeEdit() == GetFocus())
            iOutput = SendMessage(getActiveDocument()->hWnd, UM_QUERY_DOCUMENT_COMMAND, IDM_EDIT_COMMENT, NULL);
         break;

      /// [WINDOW MENU] Handle the document menu or pass to the document
      default:
         // [CHECK] Only remaining commands should be the documents of the window menu. An error here indicates you've missed something
         ASSERT(iCommandID >= IDM_WINDOW_FIRST_DOCUMENT AND iCommandID <= IDM_WINDOW_LAST_DOCUMENT);

         // Lookup the DOCUMENT matching the menu item, mark the active document with a tick
         if (findDocumentByIndex(pWindowData->hDocumentsTab, iCommandID - IDM_WINDOW_FIRST_DOCUMENT, pDocument))
            iOutput = (pDocument == getActiveDocument() ? MF_CHECKED : MF_UNCHECKED);

         // [CHECK] Document should always be found
         ASSERT(pDocument);
         break;
      }
      break;
   }

   // Return result
   return iOutput;
}


/// Function name  : loadDocumentSession
// Description     : Loads the StoredDocuments saved in the registry, if any
// 
// MAIN_WINDOW_DATA*  pWindowData : [in] Main window data
// 
VOID   loadDocumentSession(MAIN_WINDOW_DATA*  pWindowData)
{
   DOCUMENT_TYPE    eDocumentType;    // Document file type
   TCHAR           *szFullPath,       // Full path to document/project
                   *szSubKey;         // Name of the sub-key currently being created
   HKEY             hDocumentKey,     // Sub-key used to access the current document
                    hProjectKey;      // Sub-key used to access the stored project

   // Prepare
   TRACK_FUNCTION();
   szFullPath = utilCreateEmptyString(MAX_PATH);
   szSubKey   = utilCreateEmptyString(32);

   // Generate project sub-key
   StringCchPrintf(szSubKey, 32, TEXT("%s\\Project"), getAppRegistrySubKey(ARK_SESSION));

   // [CHECK] Is there a stored project?
   if (hProjectKey = utilRegistryOpenAppSubKey(getAppRegistryKey(), szSubKey))
   {
      /// [SUCCESS] Extract path and load project
      utilRegistryReadString(hProjectKey, TEXT("szFullPath"), szFullPath, MAX_PATH);
      commandLoadDocument(pWindowData, FIF_PROJECT, szFullPath, FALSE, NULL);
      
      // Cleanup
      utilRegistryCloseKey(hProjectKey);
   }

   /// [DOCUMENTS] Iterate through up to 50 possible session documents
   for (UINT  iIndex = 0; iIndex < 50; iIndex++)
   {
      // Generate document sub-key
      StringCchPrintf(szSubKey, 32, TEXT("%s\\Document%02u"), getAppRegistrySubKey(ARK_SESSION), iIndex);

      /// [CHECK] Is there another stored document?
      if (hDocumentKey = utilRegistryOpenAppSubKey(getAppRegistryKey(), szSubKey))
      {
         // Read document properties from registry
         utilRegistryReadNumber(hDocumentKey, TEXT("eType"), &eDocumentType);
         utilRegistryReadString(hDocumentKey, TEXT("szFullPath"), szFullPath, MAX_PATH);

         /// Load current document
         commandLoadDocument(pWindowData, calculateFileTypeFromDocumentType(eDocumentType), szFullPath, FALSE, NULL);
         
         // Cleanup
         utilRegistryCloseKey(hDocumentKey);
      }
      // [FAILED] Stop searching for documents
      else
         break;
   }

   // Cleanup
   utilDeleteStrings(szFullPath, szSubKey);
   END_TRACKING();
}


/// Function name  : performMainWindowReInitialisation
// Description     : Closes all documents, Unloads the GameData then reloads it if possible
///                              Requires no currently modified documents
// 
// MAIN_WINDOW_DATA*  pWindowData : [in]           Main window data
// CONST UINT         iTestCaseID : [in][optional] TestCase Number
// 
VOID  performMainWindowReInitialisation(MAIN_WINDOW_DATA*  pWindowData, CONST UINT  iTestCaseID)
{
   // [TRACK]
   TRACK_FUNCTION();

   // [CHECK] Ensure there are no MODIFIED documents
   ASSERT(!isAnyDocumentModified(pWindowData->hDocumentsTab));

   /// Store document session in the registry
   storeDocumentSession(pWindowData);

   // Close all documents and the project
   closeAllDocuments(pWindowData->hDocumentsTab, FALSE);
   closeActiveProject(NULL);

   // [CHECK] Ensure documents are closed
   if (!getDocumentCount())
   {
      /// [SUCCESS] Destroy game data
      destroyGameData();

      // [STATE] Set application state to 'NO GAME DATA'
      setMainWindowState(AS_NO_GAME_DATA);

      // [OUTPUT] Clear output window
      clearOutputDialogText(pWindowData->hOutputDlg);

      /// [CHECK] Is there a valid game data folder?
      if (getAppPreferences()->eGameFolderState == GFS_VALID)
         // [SUCCESS] Attempt to reload game data
         commandLoadGameData(pWindowData, iTestCaseID);
   }

   // [TRACK]
   END_TRACKING();
}


/// Function name  : setMainWindowTutorialTimer
// Description     : Creates or destroys the specified tutorial delayed display timer
// 
// MAIN_WINDOW_DATA*      pWindowData : [in] Window data
// CONST TUTORIAL_WINDOW  eTutorial   : [in] Tutorial window to display
// CONST BOOL             bInitiate   : [in] TRUE to create, FALSE to destroy
// 
VOID  setMainWindowTutorialTimer(MAIN_WINDOW_DATA*  pWindowData, CONST TUTORIAL_WINDOW  eTutorial, CONST BOOL  bInitiate)
{
   if (bInitiate)
      // [CREATE]
      SetTimer(pWindowData->hMainWnd, eTutorial, 250, NULL);
   else
      // [DESTROY]
      KillTimer(pWindowData->hMainWnd, eTutorial);
}



/// Function name  : storeDocumentSession
// Description     : Saves the names of the currently open documents into the registry
// 
// MAIN_WINDOW_DATA*  pWindowData : [in] Main window data
//
VOID  storeDocumentSession(MAIN_WINDOW_DATA*  pWindowData)
{
   DOCUMENT*   pDocument;        // Document currently being processed
   TCHAR*      szSubKey;         // Name of the sub-key currently being created
   HKEY        hDocumentKey,     // Sub-key used to store the current document
               hProjectKey;      // Sub-key used to store the current project

   // Prepare
   TRACK_FUNCTION();
   szSubKey = utilCreateEmptyString(32);

   /// Delete entire previous session (if any)
   utilRegistryDeleteAppSubKey(getAppRegistryKey(), getAppRegistrySubKey(ARK_SESSION));

   /// Iterate through open documents (if any)
   for (UINT  iIndex = 0; findDocumentByIndex(pWindowData->hDocumentsTab, iIndex, pDocument); iIndex++)
   {
      // [CHECK] Do not save 'Untitled' documents
      if (pDocument->bUntitled)
         continue;

      // Generate sub-key
      StringCchPrintf(szSubKey, 32, TEXT("%s\\Document%02u"), getAppRegistrySubKey(ARK_SESSION), iIndex);

      // Create subkey
      if (hDocumentKey = utilRegistryCreateAppSubKey(getAppRegistryKey(), szSubKey))
      {
         /// [SUCCESS] Store current document into registry
         utilRegistryWriteNumber(hDocumentKey, TEXT("eType"), pDocument->eType);
         utilRegistryWriteString(hDocumentKey, TEXT("szFullPath"), getDocumentPath(pDocument));
         
         // Cleanup
         utilRegistryCloseKey(hDocumentKey);
      }
   }

   // [CHECK] Is there an active project?
   if ((pDocument = getActiveProject()) AND !pDocument->bUntitled)
   {
      // Generate project sub-key
      StringCchPrintf(szSubKey, 32, TEXT("%s\\Project"), getAppRegistrySubKey(ARK_SESSION));

      // Create sub-key
      if (hProjectKey = utilRegistryCreateAppSubKey(getAppRegistryKey(), szSubKey))
      {
         /// [SUCCESS] Store path and cleanup
         utilRegistryWriteString(hProjectKey, TEXT("szFullPath"), getDocumentPath(pDocument));
         utilRegistryCloseKey(hProjectKey);
      }
   }
   
   // Cleanup
   utilDeleteString(szSubKey);
   END_TRACKING();
}


/// Function name  : updateMainWindowTitle
// Description     : Updates the main window title to reflect the current project state
// 
// MAIN_WINDOW_DATA*  pWindowData   : [in] Window data
// 
VOID  updateMainWindowTitle(MAIN_WINDOW_DATA*  pWindowData)
{
   PROJECT_DOCUMENT*  pProject;        // Current project, if any
   TCHAR*    szWindowTitle;   // New title

   // [CHECK] Is there a current project?
   if (pProject = getActiveProject())
   {
      /// [NEW PROJECT] Display "<App Name> - <Project Name> <Project modified flag>"
      szWindowTitle = utilCreateStringf(MAX_PATH, pProject AND pProject->bModified ? TEXT("%s - %s*") : TEXT("%s - %s"), getAppName(), getDocumentFileName(pProject));
      // Replace window text and cleanup
      SetWindowText(pWindowData->hMainWnd, szWindowTitle);
      utilDeleteString(szWindowTitle);
   }
   else
      /// [NO PROJECT] Revert to "<App Name>"
      SetWindowText(pWindowData->hMainWnd, getAppName());
}

/// /////////////////////////////////////////////////////////////////////////////////////////
///                                     MESSAGE HANDLERS
/// /////////////////////////////////////////////////////////////////////////////////////////

/// Function name  : onMainWindow_ChildException
// Description     : Displays an exception that has occurred within a child window
// 
// MAIN_WINDOW_DATA*  pWindowData  : [in] Window data
// HWND               hSourceWnd   : [in] Source window
// ERROR_STACK*       pException   : [in] Exception error
// 
VOID  onMainWindow_ChildException(MAIN_WINDOW_DATA*  pWindowData, HWND  hSourceWnd, ERROR_STACK*  pException)
{
   // Display
   printErrorToOutputDialog(pException);
   //displayErrorMessageDialog(NULL, pException, MAKEINTRESOURCE(IDS_TITLE_EXCEPTION), MDF_OK WITH MDF_ERROR);

   // Cleanup
   deleteErrorStack(pException);
}


/// Function name  : onMainWindow_Close
// Description     : Asks the user whether to save any 'modified' documents
// 
// MAIN_WINDOW_DATA*        pWindowData  : [in] Main window data
// CONST MAIN_WINDOW_STATE  eState           : [in] Current closure state
// 
VOID  onMainWindow_Close(MAIN_WINDOW_DATA*  pWindowData, CONST MAIN_WINDOW_STATE  eState)
{
   // [TRACK]
   TRACK_FUNCTION();

   // Examine current state
   switch (eState)
   {
   /// [CLOSING] Close any open documents
   case MWS_CLOSING:
      // [CHECK] Ensure we're not already closing
      if (!isAppClosing())
      {
         // Set app closing flag
         setAppClosing(TRUE);

         // Store document session in the registry
         storeDocumentSession(pWindowData);

         // Close/Save active documents
         closeAllDocuments(pWindowData->hDocumentsTab, FALSE);    /// [EVENT] Fires UM_MAIN_WINDOW_CLOSING with state MWS_DOCUMENTS_CLOSED
      }
      break;

   /// [DOCUMENTS CLOSED] Close project
   case MWS_DOCUMENTS_CLOSED:
      // Save/Close active project
      closeActiveProject(NULL);         /// [EVENT] Fires UM_MAIN_WINDOW_CLOSING with state MWS_PROJECT_CLOSED
      break;

   /// [PROJECT CLOSED] Submit bug report if appropriate
   case MWS_PROJECT_CLOSED:
      // [CHECK] Is a bug report necessary/enabled?
      if (getAppPreferences()->bSubmitBugReports AND hasAppErrors() AND displayBugReportDialog(pWindowData->hMainWnd, FALSE) == IDOK)
      {
         // [SUBMIT] Launch the submission thread
         commandSubmitReport(pWindowData, NULL);      /// [EVENT] Fires UM_MAIN_WINDOW_CLOSING with state MWS_REPORT_SUBMITTED
         break;
      }
      // Fall through...

   /// [REPORT SUBMITTED, CANCELLED or DISABLED] Exit app
   case MWS_REPORT_SUBMITTED:
      // [MINIMIZED] Restore window before saving dialog sizes
      if (IsMinimized(pWindowData->hMainWnd))
         ShowWindow(pWindowData->hMainWnd, SW_RESTORE);
      
      // [PROPERTIES/PROJECT/OUTPUT/SEARCH] Store visiblility to preferences
      setAppPreferencesDialogVisibility(AW_PROPERTIES, pWindowData->hPropertiesSheet != NULL);
      setAppPreferencesDialogVisibility(AW_PROJECT,    pWindowData->hProjectDlg      != NULL);
      setAppPreferencesDialogVisibility(AW_OUTPUT,     pWindowData->hOutputDlg       != NULL);
      setAppPreferencesDialogVisibility(AW_SEARCH,     pWindowData->hSearchTabDlg    != NULL);

      // [OUTPUT] Save size to preferences
      if (pWindowData->hOutputDlg)
         setAppPreferencesDialogSplit(AW_OUTPUT, getWorkspaceWindowSize(pWindowData->hWorkspace, pWindowData->hOutputDlg));

      // [SEARCH] Save size to preferences
      if (pWindowData->hSearchTabDlg)
      {
         // Save size
         setAppPreferencesDialogSplit(AW_SEARCH, getWorkspaceWindowSize(pWindowData->hWorkspace, pWindowData->hSearchTabDlg));
         //VERBOSE("DEBUG: SAVING WORKSPACE SEARCH PANE = %d pixels", getWorkspaceWindowSize(pWindowData->hWorkspace, pWindowData->hSearchTabDlg));

         // Set current tab and filters
         setAppPreferencesSearchDialogTab(identifyActiveResultsDialog(pWindowData->hSearchTabDlg));
         setAppPreferencesSearchDialogFilter(RT_COMMANDS, identifyResultsDialogFilterByType(pWindowData->hSearchTabDlg, RT_COMMANDS));
         setAppPreferencesSearchDialogFilter(RT_GAME_OBJECTS, identifyResultsDialogFilterByType(pWindowData->hSearchTabDlg, RT_GAME_OBJECTS));
         setAppPreferencesSearchDialogFilter(RT_SCRIPT_OBJECTS, identifyResultsDialogFilterByType(pWindowData->hSearchTabDlg, RT_SCRIPT_OBJECTS));
      }

      // [FIND] Save size and close window
      if (pWindowData->hFindTextDlg)
      {
         GetWindowRect(pWindowData->hFindTextDlg, getAppPreferencesWindowRect(AW_FIND));
         utilDeleteWindow(pWindowData->hFindTextDlg);
      }

      // [MAIN] Save size and position to preferences
      setAppPreferencesMainWindowState(IsMaximized(pWindowData->hMainWnd));
      // [CHECK] Don't save window size if maximised
      if (!IsMaximized(pWindowData->hMainWnd))
         GetWindowRect(pWindowData->hMainWnd, getAppPreferencesWindowRect(AW_MAIN));

      /// [MAIN] Destroy main window
      DestroyWindow(pWindowData->hMainWnd);     /// BUG_FIX: Don't use utilDeleteWindow here, causes an access violation within the heap API
      break;
   }

   // [TRACK]
   END_TRACKING();
}

/// Function name  : onMainWindow_DocumentPropertyUpdated
// Description     : Update the toolbar to reflect possible changes
//
// MAIN_WINDOW_DATA*  pWindowData : [in]           Main window data
// CONST UINT         iControlID      : [in][optional] ID of the control representing the property that has changed
// 
VOID  onMainWindow_DocumentPropertyUpdated(MAIN_WINDOW_DATA*  pWindowData, CONST UINT  iControlID)
{
   /// Update toolbar
   updateMainWindowToolBar(pWindowData);
}


/// Function name  : onMainWindow_DocumentSwitched
// Description     : Updates the Toolbar to reflect new document
// 
// MAIN_WINDOW_DATA*  pWindowData : [in] Main window data
// DOCUMENT*          pNewDocument    : [in] New active document if any, otherwise NULL
// 
VOID  onMainWindow_DocumentSwitched(MAIN_WINDOW_DATA*  pWindowData, DOCUMENT*  pNewDocument)
{
   /// Update toolbar
   updateMainWindowToolBar(pWindowData);
}


/// Function name  : onMainWindow_DocumentUpdated
// Description     : Refreshes the toolbar to reflect current document state
// 
// MAIN_WINDOW_DATA*  pWindowData : [in] Main window data
// 
VOID  onMainWindow_DocumentUpdated(MAIN_WINDOW_DATA*  pWindowData)
{
   /// Update ToolBar   
   updateMainWindowToolBar(pWindowData);
}


/// Function name  : onMainWindow_OperationComplete
// Description     : Called when a thread operation is complete but before the operation displays any errors
// 
// MAIN_WINDOW_DATA*  pWindowData : [in] Data for the main BearScript window
// OPERATION_DATA*    pOperationData  : [in] Operation data
// 
VOID   onMainWindow_OperationComplete(MAIN_WINDOW_DATA*  pWindowData, OPERATION_DATA*  pOperationData)
{
   SUBMISSION_OPERATION*  pSubmissionOperation;      // Convenience pointer

   // [TRACK]
   TRACK_FUNCTION();
   VERBOSE_LIB_EVENT();

   // Examine operation
   switch (pOperationData->eType)
   {
   /// [LOAD GAME DATA] Display appropriate windows and switch to the ready state
   case OT_LOAD_GAME_DATA:
      /// [SPLASH] Close splash window
      EnableWindow(getAppWindow(), TRUE);
      utilDeleteWindow(pWindowData->hSplashWindow);

      // [STATE] Update application state
      setMainWindowState(isOperationSuccessful(pOperationData) ? AS_GAME_DATA_LOADED : AS_NO_GAME_DATA);

      /// [SUCCESS] Load document session
      if (isOperationSuccessful(pOperationData))
      {
         // [CHECK] Are we preserving the document session?
         if (getAppPreferences()->bPreserveSession)
            loadDocumentSession(pWindowData);

         // [TUTORIAL] Delay Display 'Game Data Loaded' tutorial
         setMainWindowTutorialTimer(pWindowData, TW_GAME_DATA, TRUE);

         // [DEBUG]
         DEBUG_WINDOW("LOADED:Workspace",     pWindowData->hWorkspace);
         DEBUG_WINDOW("LOADED:DocumentsCtrl", pWindowData->hDocumentsTab);
         DEBUG_WINDOW("LOADED:OutputDlg",     pWindowData->hOutputDlg);
         DEBUG_WINDOW("LOADED:ProjectDlg",    pWindowData->hProjectDlg);
         DEBUG_WINDOW("LOADED:SearchDlg",     pWindowData->hSearchTabDlg);
      }
      /// [FAILURE] Flag as application error, ensure output dialog is visible
      else
      {
         setAppError(AE_LOAD_GAME_DATA);
         displayOutputDialog(pWindowData, TRUE);
      }
      break;

   /// [BUG REPORT] Close the program, if specified
   case OT_SUBMIT_BUG_REPORT:
      // Prepare
      pSubmissionOperation = (SUBMISSION_OPERATION*)pOperationData;

      // [SUCCESS] Reset any critical errors flags
      if (isOperationSuccessful(pOperationData))
         resetAppErrors();
      
      // [PROGRAM CLOSING] Close the program
      if (isAppClosing())
         postAppClose(MWS_REPORT_SUBMITTED);
      break;

   /// [CORRECTION] Do nothing
   case OT_SUBMIT_CORRECTION:
      break;
   }

   // [TRACK]
   END_TRACKING();
}


/// Function name  : onMainWindow_PreferencesChanged
// Description     : Inform documents control
// 
// MAIN_WINDOW_DATA*  pWindowData   : [in] 
// 
VOID    onMainWindow_PreferencesChanged(MAIN_WINDOW_DATA*  pWindowData)
{
   /// Inform Documents and search results
   Preferences_Changed(pWindowData->hDocumentsTab);
   Preferences_Changed(pWindowData->hSearchTabDlg);
}


/// Function name  : onMainWindow_ShellDropFiles
// Description     : Opens files dragged into the main window from the windows shell
// 
// MAIN_WINDOW_DATA*  pWindowData : [in] Main window data
// HDROP              hDrop           : [in] Shell drag'n'drop handle
// 
VOID  onMainWindow_ShellDropFiles(MAIN_WINDOW_DATA*  pWindowData, HDROP  hDrop)
{
   TCHAR*   szFullPath;    // Path of the file currently being processed
   UINT     iFileCount;    // Number of files being dragged

   // [TRACK]
   TRACK_FUNCTION();

   // Prepare
   szFullPath = utilCreateEmptyPath();

   // Query the number of files being dropped
   iFileCount = DragQueryFile(hDrop, -1, NULL, NULL);

   /// Iterate through dropped files
   for (UINT  iFile = 0; iFile < iFileCount; iFile++)
   {
      // Extract file path
      if (DragQueryFile(hDrop, iFile, szFullPath, MAX_PATH))
         /// Identify filetype. Load file. Activate the final document.
         commandLoadDocument(pWindowData, identifyFileType(getFileSystem(), szFullPath), szFullPath, (iFile == iFileCount - 1), NULL);
   }

   // Cleanup
   DragFinish(hDrop);
   utilDeleteString(szFullPath);
   END_TRACKING();
}


/// Function name  : onMainWindow_TutorialTimer
// Description     : Displays a tutorial window
// 
// MAIN_WINDOW_DATA*      pWindowData : [in] Window data
// CONST TUTORIAL_WINDOW  eTutorial   : [in] Tutorial to display
// 
VOID  onMainWindow_TutorialTimer(MAIN_WINDOW_DATA*  pWindowData, CONST TUTORIAL_WINDOW  eTutorial)
{
   // Destroy timer
   setMainWindowTutorialTimer(pWindowData, eTutorial, FALSE);

   // [TUTORIAL] Display tutorial window
   displayTutorialDialog(utilGetTopWindow(getAppWindow()), eTutorial, FALSE);

   /// [GAME-DATA + FIRST RUN] Display preferences
   if (eTutorial == TW_GAME_FOLDER AND getAppState() == AS_FIRST_RUN)
   {
      // [VERBOSE]
      VERBOSE("Displaying Preferences dialog due to first-run state");
      VERBOSE_SMALL_PARTITION();

      // Display preferences
      displayPreferencesDialog(pWindowData->hMainWnd, PP_FOLDERS);   // Moves AppState to AS_NO_GAME_DATA
      // Save new values
      saveAppPreferences();
   }
}
