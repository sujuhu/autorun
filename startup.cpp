
BOOL EnumStartup( AUTORUN_CALLBACK pfnCallback, LPVOID lpParam )
{
	CHKey hKey;
	DWORD dwResult = RegOpenKeyEx( HKEY_CURRENT_USER, 
										"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",
										0, KEY_QUERY_VALUE, (PHKEY)&hKey );
	if(ERROR_SUCCESS != dwResult ) {
		SetLastError( dwResult );
		return FALSE;
	}

	char Buffer[512] = {0};
	DWORD cbSize = sizeof( Buffer );
	dwResult = RegQueryValueEx( hKey.GetHandle(), "Startup", NULL, NULL, (PUCHAR)Buffer, &cbSize );
	if( ERROR_SUCCESS != dwResult ) {
		SetLastError( dwResult );
		return FALSE;
	}

	char StartupDir[512] = {0};
	ExpandEnvironmentStrings( Buffer, StartupDir, sizeof( StartupDir ) - 1 );

	char Keyword[512] = {0};
	strncpy( Keyword, StartupDir, sizeof( Keyword) - 1 );
	PathAddBackslash( Keyword );
	strncat( Keyword, "*.lnk", sizeof( Keyword ) - strlen( Keyword ) - 1  );

	//枚举文件数量
	WIN32_FIND_DATA FindFileData;
	ZeroMemory( &FindFileData, sizeof( WIN32_FIND_DATA ) );
	HANDLE hFind = FindFirstFile( Keyword, &FindFileData );
	if( hFind == INVALID_HANDLE_VALUE )
		return FALSE;
	
	do   {
		if( strncmp( FindFileData.cFileName, "..", 2 ) == 0 
			|| strncmp( FindFileData.cFileName, ".", 1 ) == 0 )
			continue;
		
		//判断是否目录
		if( FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			continue;
		
		//获取文件全路径
		AUTORUN_ITEM Item = {0};
		char LinkFile[512] = {0};
		strncpy( Item.Name, FindFileData.cFileName, sizeof( Item.Name ) - 1 );
		strncpy( LinkFile, StartupDir, sizeof( LinkFile ) - 1 );
		PathAddBackslash( LinkFile );
		strncat( LinkFile, FindFileData.cFileName, sizeof( LinkFile ) - strlen( LinkFile ) - 1 );
		GetLinkInfo( NULL, LinkFile, Item.ImagePath, NULL );
		GetLongPathName( Item.ImagePath, Item.ImagePath, sizeof( Item.ImagePath ) - 1 );
		if( !pfnCallback( AUTORUN_STARTUP, &Item, lpParam )) {
			SetLastError( ERROR_CANCELLED );
			return FALSE;
		}
	}while( FindNextFile( hFind, &FindFileData ) != 0 );

	FindClose( hFind );

	return TRUE;
}