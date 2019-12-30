#include "ioDev.h"
#include "string.h"

static pBLE_Dev g_ptIODevHead;

int RegisterIODev(pBLE_Dev ptIODev)
{
	if( ptIODev == NULL)
	{
		return -1;
	}
	g_ptIODevHead = ptIODev;

	return 0;
}

void ShowIODevs(void)
{
	pBLE_Dev ptTmp = g_ptIODevHead;
	printf("communication dev : %s\n", ptTmp->name);
}

pBLE_Dev getIODev(void)
{
    return g_ptIODevHead;
}

