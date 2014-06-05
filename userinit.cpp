
BOOL EnumUserinit( AUTORUN_CALLBACK pfnCallback, LPVOID lpParam )
{
	CHKey hKey;
	DWORD dwResult = RegOpenKeyEx( HKEY_LOCAL_MACHINE, 
										"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon", 
										0, KEY_QUERY_VALUE, (PHKEY)&hKey );
	if( ERROR_SUCCESS != dwResult ) {
		SetLastError( dwResult );
		return FALSE;
	}

	char szBuffer[2048]={0};
	DWORD cbBufSize = sizeof( szBuffer );
	dwResult = RegQueryValueEx( hKey.GetHandle(), "Userinit", NULL, NULL, (UCHAR*)szBuffer, &cbBufSize );
	if( ERROR_SUCCESS != dwResult ) {
		SetLastError( dwResult );
		return FALSE;
	}

	//计算项数量	
	char aInit[16][512] = {0};
	int nRow = 16;
	if( !SplitString( szBuffer, ',', (char*)aInit, 512, &nRow ) ) 
		return FALSE;

	for( int i=0; i < nRow; i++ ) {
		AUTORUN_ITEM Item = {0};
		strncpy( Item.ImagePath, aInit[i], sizeof( Item.ImagePath ) - 1 );
		
		if( !pfnCallback( AUTORUN_USERINIT, &Item, lpParam )) {
			SetLastError( ERROR_CANCELLED );
			return FALSE;
		}		
	}
	
	return TRUE;
}

