/*
Bonsai plugin for Miranda IM

Copyright � 2006 Cristian Libotean

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "utils.h"

int LogInit()
{
#ifdef USE_LOG
	//FILE *fout = fopen(LOG_FILE, "wt");
	//fclose(fout);
	Log("********************** Miranda started **********************");
#endif
	return 0;
}

int Log(char *format, ...)
{
#ifdef USE_LOG
	char		str[4096];
	va_list	vararg;
	int tBytes;
	FILE *fout = fopen(LOG_FILE, "at");
	if (!fout)
		{
//			MessageBox(0, "can't open file", NULL, MB_OK);
			return -1;
		}
	time_t tNow = time(NULL);
	struct tm *now = localtime(&tNow);
	strftime(str, sizeof(str), "%d %b %Y @ %H:%M:%S: ", now);
	fputs(str, fout);
	va_start(vararg, format);
	
	tBytes = _vsnprintf(str, sizeof(str), format, vararg);
	if (tBytes > 0)
		{
			str[tBytes] = 0;
		}

	va_end(vararg);
	if (str[strlen(str) - 1] != '\n')
		{
			strcat(str, "\n");
		}
	fputs(str, fout);
	fclose(fout);
#endif
	return 0;
}

int Info(char *title, char *format, ...)
{
	char str[4096];
	va_list vararg;
	int tBytes;
	va_start(vararg, format);
	tBytes = _vsnprintf(str, sizeof(str), format, vararg);
	if (tBytes > 0)
		{
			str[tBytes] = 0;
		}
	va_end(vararg);
	return MessageBoxA(0, str, title, MB_OK | MB_ICONINFORMATION);		
}

int MyPUShowMessage(char *lpzText, BYTE kind)
{
	if (ServiceExists(MS_POPUP_SHOWMESSAGE))
	{
		return PUShowMessage(lpzText, kind);
	}
	else{
		char *title = (kind == SM_NOTIFY) ? Translate("Notify") : Translate("Warning");
		
		return MessageBox(NULL, lpzText, title, MB_OK | (kind == SM_NOTIFY) ? MB_ICONINFORMATION : MB_ICONEXCLAMATION);
	}
}

#define HEX_SIZE 8

char *BinToHex(int size, PBYTE data)
{
	char *szresult = NULL;
	char buffer[32] = {0}; //should be more than enough
	int maxSize = size * 2 + HEX_SIZE + 1;
	szresult = (char *) new char[ maxSize ];
	memset(szresult, 0, maxSize);
	sprintf(buffer, "%0*X", HEX_SIZE, size);
	strcpy(szresult, buffer);
	int i;
	for (i = 0; i < size; i++)
		{
			sprintf(buffer, "%02X", data[i]);
			strcpy(szresult + (HEX_SIZE + i * 2), buffer);
		}
	return szresult; 
}

void HexToBin(char *inData, ULONG &size, LPBYTE &outData)
{
	char buffer[32] = {0};
	strcpy(buffer, "0x");
	strncpy(buffer + 2, inData, HEX_SIZE);
	sscanf(buffer, "%x", &size);
	outData = (unsigned char*)new char[size*2];
	UINT i;
	//size = i;
	char *tmp = inData + HEX_SIZE;
	buffer[4] = '\0'; //mark the end of the string
	for (i = 0; i < size; i++)
		{
			strncpy(buffer + 2, &tmp[i * 2], 2);
			sscanf(buffer, "%x", &outData[i]);
		}
	i = size;
}

int GetStringFromDatabase(HANDLE hContact, char *szModule, char *szSettingName, char *szError, char *szResult, size_t size)
{
	DBVARIANT dbv = {0};
	int res = 1;
	size_t len;
	dbv.type = DBVT_ASCIIZ;
	if (DBGetContactSetting(hContact, szModule, szSettingName, &dbv) == 0)
		{
			res = 0;
			size_t tmp = strlen(dbv.pszVal);
			len = (tmp < size - 1) ? tmp : size - 1;
			strncpy(szResult, dbv.pszVal, len);
			szResult[len] = '\0';
			MirandaFree(dbv.pszVal);
		}
		else{
			res = 1;
			if (szError)
				{
					size_t tmp = strlen(szError);
					len = (tmp < size - 1) ? tmp : size - 1;
					strncpy(szResult, szError, len);
					szResult[len] = '\0';
				}
				else{
					szResult[0] = '\0';
				}
		}
	return res;
}

int GetStringFromDatabase(HANDLE hContact, char *szModule, char *szSettingName, WCHAR *szError, WCHAR *szResult, size_t count)
{
	DBVARIANT dbv = {0};
	int res = 1;
	size_t len;
	dbv.type = DBVT_WCHAR;
	if (DBGetContactSettingWString(hContact, szModule, szSettingName, &dbv) == 0)
		{
			res = 0;
			if (dbv.type != DBVT_WCHAR)
			{
				MultiByteToWideChar(CP_ACP, 0, dbv.pszVal, -1, szResult, (int) count);
			}
			else{
				size_t tmp = wcslen(dbv.pwszVal);
				len = (tmp < count - 1) ? tmp : count - 1;
				wcsncpy(szResult, dbv.pwszVal, len);
				szResult[len] = L'\0';
			}
			MirandaFree(dbv.pwszVal);
		}
		else{
			res = 1;
			if (szError)
				{
					size_t tmp = wcslen(szError);
					len = (tmp < count - 1) ? tmp : count - 1;
					wcsncpy(szResult, szError, len);
					szResult[len] = L'\0';
				}
				else{
					szResult[0] = L'\0';
				}
		}
	return res;
}

int GetStringFromDatabase(char *szSettingName, char *szError, char *szResult, size_t size)
{
	return GetStringFromDatabase(NULL, ModuleName, szSettingName, szError, szResult, size);
}

int GetStringFromDatabase(char *szSettingName, WCHAR *szError, WCHAR *szResult, size_t count)
{
	return GetStringFromDatabase(NULL, ModuleName, szSettingName, szError, szResult, count);
}

#pragma warning (disable: 4312) 
TCHAR *GetContactName(HANDLE hContact, char *szProto)
{
	CONTACTINFO ctInfo;
	int ret;
	char proto[200];
	
	ZeroMemory((void *) &ctInfo, sizeof(ctInfo));	
	ctInfo.cbSize = sizeof(ctInfo);
	if (szProto)
		{
			ctInfo.szProto = szProto;
		}
		else{
			GetContactProtocol(hContact, proto, sizeof(proto));
			ctInfo.szProto = proto;
		}
	ctInfo.dwFlag = CNF_DISPLAY;
#ifdef _UNICODE
	ctInfo.dwFlag += CNF_UNICODE;
#endif	
	ctInfo.hContact = hContact;
	//_debug_message("retrieving contact name for %d", hContact);
	ret = CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM) &ctInfo);
	//_debug_message("	contact name %s", ctInfo.pszVal);
	TCHAR *buffer;
	if (!ret)
		{
			buffer = _tcsdup(ctInfo.pszVal);
		}
	MirandaFree(ctInfo.pszVal);
	if (!ret)
		{
			return buffer;
		}
		else{
			return NULL;
		}
	return buffer;
}
#pragma warning (default: 4312)

#pragma warning (disable: 4312) 
void GetContactProtocol(HANDLE hContact, char *szProto, size_t size)
{
	GetStringFromDatabase(hContact, "Protocol", "p", NULL, szProto, size);
}
#pragma warning (default: 4312)

#pragma warning (disable: 4312)
TCHAR *GetContactID(HANDLE hContact)
{
	char protocol[256];
	GetContactProtocol(hContact, protocol, sizeof(protocol));

	return GetContactID(hContact, protocol);
}

TCHAR *GetContactID(HANDLE hContact, char *szProto)
{
	CONTACTINFO ctInfo;
	int ret;

	ZeroMemory((void *) &ctInfo, sizeof(ctInfo));	
	ctInfo.cbSize = sizeof(ctInfo);
	ctInfo.szProto = szProto;
	ctInfo.dwFlag = CNF_UNIQUEID;
#ifdef _UNICODE
	ctInfo.dwFlag |= CNF_UNICODE;
#endif
	ctInfo.hContact = hContact;
	ret = CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM) &ctInfo);
	TCHAR *buffer;
	if (!ret)
		{
			TCHAR tmp[16];
			switch (ctInfo.type)
				{
					case CNFT_BYTE:
						{
							_stprintf(tmp, _T("%d"), ctInfo.bVal);
							buffer = _tcsdup(tmp);
						
							break;
						}
						
					case CNFT_WORD:
						{
							_stprintf(tmp, _T("%d"), ctInfo.wVal);
							buffer = _tcsdup(tmp);
						
							break;
						}
						
					case CNFT_DWORD:
						{
							_stprintf(tmp, _T("%ld"), ctInfo.dVal);
							buffer = _tcsdup(tmp);
							
							break;
						}
						
					case CNFT_ASCIIZ:
					default:
						{
							buffer = _tcsdup(ctInfo.pszVal);
							
							break;
						}
				}
				

		}
	MirandaFree(ctInfo.pszVal);
	if (!ret)
		{
			return buffer;
		}
		else{
			return NULL;
		}
}
#pragma warning (default: 4312)

#pragma warning (disable: 4312)
HANDLE GetContactFromID(TCHAR *szID, char *szProto)
{
	HANDLE hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	TCHAR *szHandle;
	char cProtocol[256];

	int found = 0;
	while (hContact)
		{
			GetContactProtocol(hContact, cProtocol, sizeof(cProtocol));
			szHandle = GetContactID(hContact, cProtocol);
			if ((szHandle) && (_tcsicmp(szHandle, szID) == 0) && (_stricmp(szProto, cProtocol) == 0))
				{
					found = 1;
				}
			if (szHandle) free(szHandle);
		
			if (found)
				{
					break;
				}
			hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
		}
	return hContact;
}
#pragma warning (default: 4312)

#pragma warning (disable: 4312)
HANDLE GetContactFromID(TCHAR *szID, wchar_t *szProto)
{
	char protocol[1024];
	WideCharToMultiByte(CP_ACP, 0, szProto, -1, protocol, sizeof(protocol), NULL, NULL);
	return GetContactFromID(szID, protocol);
}
#pragma warning (default: 4312)

void ScreenToClient(HWND hWnd, LPRECT rect)
{
	POINT pt;
	int cx = rect->right - rect->left;
	int cy = rect->bottom - rect->top;
	pt.x = rect->left;
	pt.y = rect->top;
	ScreenToClient(hWnd, &pt);
	rect->left = pt.x;
	rect->top = pt.y;
	rect->right = pt.x + cx;
	rect->bottom = pt.y + cy;
}

void AnchorMoveWindow(HWND window, const WINDOWPOS *parentPos, int anchors)
{
	RECT rParent;
	RECT rChild;
	
	if (parentPos->flags & SWP_NOSIZE)
		{
			return;
		}
	GetWindowRect(parentPos->hwnd, &rParent);
	rChild = AnchorCalcPos(window, &rParent, parentPos, anchors);
	MoveWindow(window, rChild.left, rChild.top, rChild.right - rChild.left, rChild.bottom - rChild.top, FALSE);
}

RECT AnchorCalcPos(HWND window, const RECT *rParent, const WINDOWPOS *parentPos, int anchors)
{
	RECT rChild;
	RECT rTmp;

	GetWindowRect(window, &rChild);
	ScreenToClient(parentPos->hwnd, &rChild);

	int cx = rParent->right - rParent->left;
	int cy = rParent->bottom - rParent->top;
	if ((cx == parentPos->cx) && (cy == parentPos->cy))
		{
			return rChild;
		}
	if (parentPos->flags & SWP_NOSIZE)
		{
			return rChild;
		}

	rTmp.left = parentPos->x - rParent->left;
	rTmp.right = (parentPos->x + parentPos->cx) - rParent->right;
	rTmp.bottom = (parentPos->y + parentPos->cy) - rParent->bottom;
	rTmp.top = parentPos->y - rParent->top;
	
	cx = (rTmp.left) ? -rTmp.left : rTmp.right;
	cy = (rTmp.top) ? -rTmp.top : rTmp.bottom;	
	
	rChild.right += cx;
	rChild.bottom += cy;
	//expanded the window accordingly, now we need to enforce the anchors
	if ((anchors & ANCHOR_LEFT) && (!(anchors & ANCHOR_RIGHT)))
		{
			rChild.right -= cx;
		}
	if ((anchors & ANCHOR_TOP) && (!(anchors & ANCHOR_BOTTOM)))
		{
			rChild.bottom -= cy;
		}
	if ((anchors & ANCHOR_RIGHT) && (!(anchors & ANCHOR_LEFT)))
		{
			rChild.left += cx;
		}
	if ((anchors & ANCHOR_BOTTOM) && (!(anchors & ANCHOR_TOP)))
		{
			rChild.top += cy;
		}
	return rChild;
}

DWORD NameHashFunction(const char *szStr)
{
#if defined _M_IX86 && !defined _NUMEGA_BC_FINALCHECK && !defined NOINLINEASM
	__asm {		   //this breaks if szStr is empty
		xor  edx,edx
		xor  eax,eax
		mov  esi,szStr
		cmp esi, 0x0
		je end
		mov  al,[esi]
		xor  cl,cl
	lph_top:	 //only 4 of 9 instructions in here don't use AL, so optimal pipe use is impossible
		xor  edx,eax
		inc  esi
		xor  eax,eax
		and  cl,31
		mov  al,[esi]
		add  cl,5
		test al,al
		rol  eax,cl		 //rol is u-pipe only, but pairable
		                 //rol doesn't touch z-flag
		jnz  lph_top  //5 clock tick loop. not bad.

		xor  eax,edx
		end:
	}
#else
	DWORD hash=0;
	int i;
	int shift=0;
	for(i=0;szStr[i];i++) {
		hash^=szStr[i]<<shift;
		if(shift>24) hash^=(szStr[i]>>(32-shift))&0x7F;
		shift=(shift+5)&0x1F;
	}
	return hash;
#endif
}

DWORD LangPackHashW(const WCHAR *szStr)
{
#if defined _M_IX86 && !defined _NUMEGA_BC_FINALCHECK && !defined __GNUC__
	__asm {				//this is mediocrely optimised, but I'm sure it's good enough
		xor  edx,edx
		xor eax, eax
		mov  esi,szStr
		cmp esi, 0x0
		je end
		xor  cl,cl
lph_top:
		xor  eax,eax
		and  cl,31
		mov  al,[esi]
		inc  esi
		inc  esi
		test al,al
		jz   lph_end
		rol  eax,cl
		add  cl,5
		xor  edx,eax
		jmp  lph_top
lph_end:
		mov  eax,edx
end:
	}
#else
	DWORD hash=0;
	int i;
	int shift=0;
	for(i=0;szStr[i];i+=2) {
		hash^=szStr[i]<<shift;
		if(shift>24) hash^=(szStr[i]>>(32-shift))&0x7F;
		shift=(shift+5)&0x1F;
	}
	return hash;
#endif
}