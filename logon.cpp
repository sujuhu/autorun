#include <windows.h>
#include "logon.h"

typedef struct _inline_run_t
{
	run_t data;
	snode_t node;
}inline_run_t;

typedef struct _logon_t
{
	slist_t run_list;
}logon_t;

BOOL logon_open()
{	
	CHKey hKey;
	DWORD dwResult = RegOpenKeyEx( HKEY_LOCAL_MACHINE, 
		"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
		0, KEY_QUERY_VALUE, (PHKEY)&hKey );
	if(ERROR_SUCCESS != dwResult ) {
		SetLastError( dwResult );
		return INVALID_LOGON;
	}

	DWORD cValue = 0;
	dwResult = RegQueryInfoKey(hKey.GetHandle(), NULL, NULL, 
		0, NULL, NULL,NULL, &cValue, NULL, NULL, NULL, NULL );
	if( ERROR_SUCCESS != dwResult ) {
		SetLastError( dwResult );
		return INVALID_LOGON;
	}

	logon_t* logon = (logon_t*)malloc(sizeof(logon_t));
	if( logon == NULL) {
		return INVALID_LOGON;
	}
	memset(logon, 0, sizeof(logon_t));

	for( DWORD i=0; i < cValue; i++ ) {
		inline_run_t *item = (inline_run_t*)malloc(sizeof(inline_run_t));
		if (item == NULL) {
			return INVALID_LOGON;
		}
		memset(item, 0, sizeof(inline_run_t));

		DWORD	cbNameSize = sizeof(item->data.name);
		char	Value[512] = {0};
		DWORD	cbValueSize = sizeof(item->data.image_path);
		dwResult = RegEnumValue(hKey.GetHandle(), i, item->data.name, 
			&cbNameSize, 0, NULL, (uint8_t*)Value, &cbValueSize);
		if (dwResult != ERROR_SUCCESS && ERROR_NO_MORE_ITEMS != dwResult) {
			SetLastError( dwResult );
			return FALSE;
		}

		char szImagePath[512] = {0};
		ParseCmdLine(Value, szImagePath, sizeof(szImagePath));
		GetLongPathName(szImagePath, item->image_path, sizeof(item->image_path) -1);
		slist_add(&logon->run_list, &item->node);
	}

	return (int)logon;
}

run_t* logon_first(int fd)
{
	if (fd == INVALID_LOGON) {
		errno = EINVAL;
		return NULL;
	}

	logon_t* logon = (logon_t*)fd;
	return slist_first_entry(&logon->run_list, inline_run_t, data, node);
}

run_t* logon_next(run_t* iter)
{
	if (iter == NULL) {
		errno = EINVAL;
		return NULL;
	}
	return slist_next_entry(iter, inline_run_t, data, node)
}

void logon_close(int fd)
{
	if (fd == INVALID_LOGON) {
		errno = EINVAL;
		return;
	}

	logon_t* logon = (logon_t*)fd;
	inline_run_t * run = NULL;
	slist_for_each_safe(run, &logon->run_list, inline_run_t, data, node) {
		free(run);
		run = NULL;
	}
	free(logon);
}