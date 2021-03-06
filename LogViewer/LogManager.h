#pragma once

//#ifndef UNICODE
//#  error 目前只支持Unicode编译
//#endif
#include <memory>

#include "..\LogViewerDefine.h"
#include "LogViewerConfig.h"

enum LogItemContentType  //用于排序
{
    type_Sequence = 0,
    type_Machine,
    type_ProcessId,
    type_ThreadId,
    type_Time,
    type_ElapseTime,
    type_TraceLevel,
    type_ModuleName,
    type_FunName,
    type_FileName,
    type_TraceInfo,

    type_SortCount
};

enum FilterType {
    ftAll,
    ftAny,
    ftNone
};
struct SortContent
{
    LogItemContentType  contentType;
    BOOL                bSortAscending;
};

struct LogItemFilter;

typedef std::list<CString>         SameNameFilePathList;
typedef std::shared_ptr<SameNameFilePathList>   SameNameFilePathListPtr;
typedef std::map<CString, SameNameFilePathListPtr, CAtlStringCompareI> FileName2FullPathMap;

class CLogManager: public IFileFindCallback
{
    friend struct LogItemFilter;
public:
    //IFileFindCallback
    virtual FileFindResultHandle OnFindFile(LPCTSTR pszFilePath, const WIN32_FIND_DATA& findData, LPVOID pParam);
public:
    CLogManager(void);
    ~CLogManager(void);
    //LONG GenerateLog(LPCTSTR pszFilePath, LONG logCount);
    BOOL SetLogFiles(const CStringArray &logFilePaths);
    BOOL ClearAllLogItems();
    BOOL ReloadLogItems();
    LONG GetLogFileCount() const;
    //LONG GetProcessCount() const;
    //LONG GetThreadCount() const;
    VOID GetSelectedCount(LONG& nSelectedProcess, LONG& nSelectedThread);

    //! bAll 为 TRUE 表示保存所有的日志
    //! bAll 为 FALSE 表示保存显示的日志(按照显示的顺序进行保存)
    BOOL SaveLogItems(LPCTSTR pszFilePath, BOOL bAll = FALSE);

    LONG GetDisplayLogItemCount() const;
    LONG GetTotalLogItemCount() const;

    const LogItemPointer GetDisplayLogItem(LONG index) const;
    BOOL TryReparseRealFileName(CString& strFileName);

    BOOL DeleteItems(std::set<LONG> delItems);
    void setActiveItemIndex(LONG index);
    CString getActiveItemTraceInfo();

    MachinePidTidContainer& GetAllMachinePidTidInfos(){
        return m_allMachinePidTidInfos;
    }
    //LONG GetThreadIds(ThreadIdContainer & threadIds) const;
    //LONG GetProcessIds(ProcessIdContainer & processIds) const;
    BOOL SortDisplayItem(LogItemContentType SortContentType, BOOL bSortAscending);
    SortContent GetFirstSortContent() const;

    BOOL IsItemMatchSeqNumber(LONG seqNumber);
    //是否选中 -- 进行过滤
    BOOL IsItemIdChecked(const LogItemPointer& pItem);

    void OnlySelectSpecialItems(const MachinePIdTIdType& selectIdType, ONLY_SELECT_TYPE selType);

    BOOL SetTraceLevelDisplay(TraceLevel level, BOOL bDisplay);

    BOOL DoFilterLogItems();
    void SetCodepage(UINT codepage){ m_codePage = codepage; }
    BOOL SetFilterSeqNumber(LONG nStartSeqNumber, LONG nEndSeqNumber);
    BOOL SetLogInfoFilterString(LPCTSTR pszFilterString, FilterType filterType);
    CLogViewerConfig    m_logConfig;

    BOOL NeedScanSourceFiles() {
        return m_filesMap.empty();
    }
    BOOL ScanSourceFiles(const CString& strFolderPath);
    SameNameFilePathListPtr FindFileFullPath(const CString& strFileName);
	VOID ClearUserFullPathCache();
	VOID SetFullPathForUserCache(const CString& strFileLineCache, const CString& strFullPathUserSelect);
	CString GetFullPathFromUserCache(const CString& strFileLineCache);
protected:
    typedef std::vector<LogItemPointer>     LogItemArrayType;
    typedef LogItemArrayType::iterator      LogItemArrayIterator;
    typedef std::list<LogItemPointer>       LogItemListType;
	typedef std::map<CString, CString>		UserSelectFullPathMap;
    //typedef std::back_insert_iterator< LogItemArrayType > back_ins_itr;
    
    FileName2FullPathMap        m_filesMap;

    UINT                        m_codePage;
    mutable CFCriticalSection   m_CsLockObj;
    CStringArray                m_logFilePaths;
    std::list<LogItemPointer>   m_allInitLogItems;  //初始化时用来读取LogItem，防止使用vector时频繁分配内存
    LogItemArrayType            m_AllLogItems;
    LogItemArrayType            m_DisplayLogItems;

    MachinePidTidContainer      m_allMachinePidTidInfos;
	UserSelectFullPathMap		m_userSelectFullPathMap;
    //ThreadIdContainer           m_AllLogThreadIds;
    //std::map<THREAD_ID_TYPE, BOOL>      m_AllLogThreadIdsChecked;

    //AllProcessIdContainer               m_AllLogProcessIds;
    //std::map<PROCESS_ID_TYPE, BOOL>     m_AllLogProcessIdsChecked;

    typedef  std::map<MachinePIdTIdType, LONGLONG>  ThreadExecuteTimeContainer;  //机器名+进程ID+线程ID,上一次的时间值
    ThreadExecuteTimeContainer  m_threadExecuteTimes;

    CString                     m_strLogInfoFilterString;
    FilterType                  m_filterType;
    //BOOL                      m_isIncludeText;
    LONG                        m_fileCount;
    LONG                        m_nStartSeqNumber;
    LONG                        m_nEndSeqNumber;
    LONG                        m_nSelectedProcessCount;
    LONG                        m_nSelectedThreadCount;
    LONG                        m_activeItemIndex;
    //增加了按照多个条件进行排序 -- 在有多个线程且对线程ID排序时，不这样做，各个日志可能乱序
    //是否应该在UI上增加标示？
    SortContent                 m_SortContents[type_SortCount];
    BOOL                        m_TraceLevelDisplay[tlEnd];         ///< 过滤Filter
protected:
    //typedef BOOL (__thiscall * ReadLogFileProc)(CLogManager*pThis, LPCTSTR pszFilePath);

    //BOOL CheckFTLogFiles(const CStringArray &logFilePaths);
    //BOOL ReadFTLogFile(LPCTSTR pszFilePath);
    //LogItemPointer ParseFTLLogItem(CString& strLogItem);

    //LogItemPointer ParseTraceLog(std::string& strOneLog);
    BOOL ReadTraceLogFile(LPCTSTR pszFilePath);

    LogItemPointer ParseRegularTraceLog(std::string& strOneLog, const std::tr1::regex& reg, const LogItemPointer& preLogItem);
    BOOL _CalcThreadElpaseTime(LogItemArrayType& logItems);
    int _ConvertItemInfo(const std::string& srcInfo, LPCTSTR& pszDest, UINT codePage);
    LPCTSTR _CopyItemInfo(LPCTSTR pszSource);
    void _AppendLogItem(LogItemPointer& pLogItem);
};
