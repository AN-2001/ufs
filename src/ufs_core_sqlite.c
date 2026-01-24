/******************************************************************************\
*  ufs_core_sqlite.c                                                           *
*                                                                              *
*  Sqlite implementation of ufs_core.                                          *
*                                                                              *
*              Written by A.N.                                  24-01-2026     *
*                                                                              *
\******************************************************************************/

#include "ufs_core.h"

ufsType ufsInit()
{
	return 0;
}

void ufsDestroy( ufsType ufs )
{
}

ufsIdentifierType ufsAddDirectory( ufsType ufs,
                                   const char *name )
{
	return 0;
}

ufsIdentifierType ufsAddFile( ufsType ufs,
                              ufsIdentifierType directory,     
                              const char *name )
{
	return 0;
}

ufsIdentifierType ufsAddArea( ufsType ufs,
                              const char *name )
{
	return 0;
}

ufsStatusType ufsAddMapping( ufsType ufs,
                             ufsIdentifierType area,
                             ufsIdentifierType storage )
{
	return 0;
}

ufsIdentifierType ufsGetDirectory( ufsType ufs,
                                   const char *name )
{
	return 0;
}

ufsIdentifierType ufsGetFile( ufsType ufs,
                              ufsIdentifierType directory,
                              const char *name )
{
	return 0;
}

ufsIdentifierType ufsGetArea( ufsType ufs,
                              const char *name )
{
	return 0;
}

ufsStatusType ufsProbeMapping( ufsType ufs,
                               ufsIdentifierType area,
                               ufsIdentifierType storage )
{
	return 0;
}

ufsStatusType ufsRemoveDirectory( ufsType ufs,
                                  ufsIdentifierType directory )
{
	return 0;
}

ufsStatusType ufsRemoveFile( ufsType ufs,
                             ufsIdentifierType file )
{
	return 0;
}

ufsStatusType ufsRemoveArea( ufsType ufs,
                             ufsIdentifierType area )
{
	return 0;
}

ufsStatusType ufsRemoveMapping( ufsType ufs,
                                ufsIdentifierType area,
                                ufsIdentifierType storage )
{
	return 0;
}

ufsIdentifierType ufsResolveStorageInView( ufsType ufs,
                                           ufsViewType view,
                                           ufsIdentifierType storage )
{
	return 0;
}

ufsStatusType ufsIterateDirInView( ufsType ufs,
                                   ufsViewType view,
                                   ufsIdentifierType directory,
                                   ufsDirIter iterator,
                                   void *userData )
{
	return 0;
}

ufsStatusType ufsCollapse( ufsType ufs,
                           ufsViewType view )
{
	return 0;
}
