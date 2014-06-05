


BOOL EnumNetworkProvider( AUTORUN_CALLBACK pfnCallback, LPVOID lpParam )
{
	CHKey hKey;
	DWORD dwResult = RegOpenKeyEx( HKEY_LOCAL_MACHINE, 
		"SYSTEM\\CurrentControlSet\\Control\\NetworkProvider\\Order",
		0, KEY_QUERY_VALUE, (PHKEY)&hKey );
	if(ERROR_SUCCESS != dwResult ) {
		SetLastError( dwResult );
		return FALSE;
	}

	char Buffer[1024] = {0};
	DWORD cbBuffer = sizeof( Buffer );
	dwResult = RegQueryValueEx( hKey.GetHandle(), "ProviderOrder", NULL, NULL, (UCHAR*)Buffer, &cbBuffer);
	if( ERROR_SUCCESS != dwResult ) {
		SetLastError( dwResult );
		return FALSE;
	}

	char aProvicer[64][256] = {0};
	int nRow = 32;
	if( !SplitString( Buffer, ',', (char*)aProvicer, 256, &nRow ) ) {
		SetLastError( ERROR_INSUFFICIENT_BUFFER );
		return FALSE;
	}

	for( int i=0; i < nRow; i++ ) {
		AUTORUN_ITEM Item = {0};
		strncpy( Item.Name, (LPCTSTR)aProvicer[i], sizeof( Item.Name) - 1 );
		
		char szSubKey[512] = {0};
		strncpy( szSubKey, "SYSTEM\\CurrentControlSet\\Services\\", sizeof( szSubKey ) - 1 );
		strncat( szSubKey, Item.Name, sizeof( szSubKey ) - strlen( szSubKey ) - 1 );
		strncat( szSubKey, "\\NetworkProvider", sizeof( szSubKey ) - strlen( szSubKey ) - 1 );
		
		CHKey	hSubKey;
		dwResult = RegOpenKeyEx( HKEY_LOCAL_MACHINE, szSubKey, 0, KEY_QUERY_VALUE, (PHKEY)&hSubKey );
		if( ERROR_SUCCESS != dwResult ) {
			SetLastError( dwResult );
			return FALSE;
		}
		
		char	Buffer[1024] = {0};
		DWORD	cbBuffer = sizeof( Buffer );
		dwResult = RegQueryValueEx( hSubKey.GetHandle(), "ProviderPath", 0, NULL ,(UCHAR*)Buffer, &cbBuffer );
		if( ERROR_SUCCESS != dwResult ) {
			SetLastError( dwResult );
			return FALSE;
		}
		
		//寻找环境变量
		DWORD cSize = ExpandEnvironmentStrings( Buffer, Item.ImagePath, sizeof( Item.ImagePath) - 1 );
		if( cSize > sizeof( Item.ImagePath ) - 1 )
			return FALSE;
		
		if( !pfnCallback( AUTORUN_NETWORK_PROVIDER, &Item, lpParam )) {
			SetLastError( ERROR_CANCELLED );
			return FALSE;
		}
	}

	return TRUE;
}