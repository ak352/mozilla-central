/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.0 (the "NPL"); you may not use this file except in
 * compliance with the NPL.  You may obtain a copy of the NPL at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the NPL is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the NPL
 * for the specific language governing rights and limitations under the
 * NPL.
 *
 * The Initial Developer of this code under the NPL is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation.  All Rights
 * Reserved.
 */

#include "nsMenu.h"
#include "nsIMenu.h"
#include "nsIMenuBar.h"
#include "nsIMenuItem.h"

#include "nsString.h"
#include "nsStringUtil.h"
#include "nsIMenuListener.h"

#if defined(XP_MAC)
#include <Appearance.h>
#include <TextUtils.h>
#include <ToolUtils.h>
#include <Devices.h>
#include <UnicodeConverter.h>
#include <Fonts.h>
#endif

static NS_DEFINE_IID(kISupportsIID, NS_ISUPPORTS_IID);
static NS_DEFINE_IID(kIMenuIID, NS_IMENU_IID);
static NS_DEFINE_IID(kIMenuBarIID, NS_IMENUBAR_IID);
static NS_DEFINE_IID(kIMenuItemIID, NS_IMENUITEM_IID);

#ifdef APPLE_MENU_HACK
const PRInt16 kAppleMenuID = 1;
const PRInt16 kMacMenuID = 2;
#else
const PRInt16 kMacMenuID = 1;
#endif /* APPLE_MENU_HACK */

PRInt16 mMacMenuIDCount = kMacMenuID;

nsresult nsMenu::QueryInterface(REFNSIID aIID, void** aInstancePtr)      
{                                                                        
  if (NULL == aInstancePtr) {                                            
    return NS_ERROR_NULL_POINTER;                                        
  }                                                                      
                                                                         
  *aInstancePtr = NULL;                                                  
                                                                                        
  if (aIID.Equals(kIMenuIID)) {                                         
    *aInstancePtr = (void*)(nsIMenu*) this;                                        
    NS_ADDREF_THIS();                                                    
    return NS_OK;                                                        
  }                                                                      
  if (aIID.Equals(kISupportsIID)) {                                      
    *aInstancePtr = (void*)this;                        
    NS_ADDREF_THIS();                                                    
    return NS_OK;                                                        
  }
  if (aIID.Equals(kIMenuListenerIID)) {                                      
    *aInstancePtr = (void*) ((nsIMenuListener*)this);                        
    NS_ADDREF_THIS();                                                    
    return NS_OK;                                                        
  }                                                     
  return NS_NOINTERFACE;                                                 
}

NS_IMPL_ADDREF(nsMenu)
NS_IMPL_RELEASE(nsMenu)

//-------------------------------------------------------------------------
//
// nsMenu constructor
//
//-------------------------------------------------------------------------
nsMenu::nsMenu() : nsIMenu()
{
  NS_INIT_REFCNT();
  mNumMenuItems  = 0;
  mMenuParent    = nsnull;
  mMenuBarParent = nsnull;
  
  mMacMenuID = 0;
  mMacMenuHandle = nsnull;
  mListener      = nsnull;
  
  //
  // create a multi-destination Unicode converter which can handle all of the installed
  //	script systems
  //
  OSErr err = ::CreateUnicodeToTextRunInfoByScriptCode(0,NULL,&mUnicodeTextRunConverter);
  NS_ASSERTION(err==noErr,"nsMenu::nsMenu: CreateUnicodeToTextRunInfoByScriptCode failed.");	

}

//-------------------------------------------------------------------------
//
// nsMenu destructor
//
//-------------------------------------------------------------------------
nsMenu::~nsMenu()
{
  OSErr		err;
  NS_IF_RELEASE(mListener);

  while(mNumMenuItems)
  {
    --mNumMenuItems;
    
    if(mMenuItemVoidArray[mNumMenuItems]) {
      // Figure out what we're releasing
      nsIMenuItem * menuitem = nsnull;
      ((nsISupports*)mMenuItemVoidArray[mNumMenuItems])->QueryInterface(kIMenuItemIID, (void**) &menuitem);  
      if(menuitem)
      {
        // case menuitem
        menuitem->Release(); // Release our hold
        NS_IF_RELEASE(menuitem); // Balance QI
      }
      else
      {
	    nsIMenu * menu = nsnull;
	    ((nsISupports*)mMenuItemVoidArray[mNumMenuItems])->QueryInterface(kIMenuIID, (void**) &menu);
	    if(menu)
	    {
	      // case menu
	      menu->Release(); // Release our hold 
	      NS_IF_RELEASE(menu); // Balance QI
	    }
	  }
	}
  }
  
  err = ::DisposeUnicodeToTextRunInfo(&mUnicodeTextRunConverter);
  NS_ASSERTION(err==noErr,"nsMenu::~nsMenu: DisposeUnicodeToTextRunInfo failed.");	  
}

//-------------------------------------------------------------------------
//
// Create the proper widget
//
//-------------------------------------------------------------------------
NS_METHOD nsMenu::Create(nsISupports *aParent, const nsString &aLabel)
{
  if(aParent)
  {
    nsIMenuBar * menubar = nsnull;
    aParent->QueryInterface(kIMenuBarIID, (void**) &menubar);
    if(menubar)
    {
      mMenuBarParent = menubar;;
      NS_RELEASE(menubar); // Balance the QI
    }
    else
    {
      nsIMenu * menu = nsnull;
      aParent->QueryInterface(kIMenuIID, (void**) &menu);
      {
      	mMenuParent = menu;
      	NS_RELEASE(menu); // Balance the QI
      }
    }
  }
  
  return NS_OK;
}

//-------------------------------------------------------------------------
NS_METHOD nsMenu::GetParent(nsISupports*& aParent)
{

  aParent = nsnull;
  if (nsnull != mMenuParent) {
    return mMenuParent->QueryInterface(kISupportsIID,(void**)&aParent);
  } else if (nsnull != mMenuBarParent) {
    return mMenuBarParent->QueryInterface(kISupportsIID,(void**)&aParent);
  }

  return NS_ERROR_FAILURE;
}

//-------------------------------------------------------------------------
NS_METHOD nsMenu::GetLabel(nsString &aText)
{
  aText = mLabel;
  return NS_OK;
}

//-------------------------------------------------------------------------
NS_METHOD nsMenu::SetLabel(const nsString &aText)
{
  mLabel = aText;

  mMacMenuHandle = NSStringNewMenu(mMacMenuIDCount,mLabel);
  mMacMenuID = mMacMenuIDCount;
  mMacMenuIDCount++;

  return NS_OK;
}

//-------------------------------------------------------------------------
NS_METHOD nsMenu::GetAccessKey(nsString &aText)
{
  return NS_OK;
}

//-------------------------------------------------------------------------
NS_METHOD nsMenu::SetAccessKey(const nsString &aText)
{
  return NS_OK;
}

//-------------------------------------------------------------------------
NS_METHOD nsMenu::AddItem(nsISupports* aItem)
{
  if(aItem)
  {
    // Figure out what we're adding
    nsIMenuItem * menuitem = nsnull;
    aItem->QueryInterface(kIMenuItemIID, (void**) &menuitem);  
    if(menuitem)
    {
      // case menuitem
      AddMenuItem(menuitem);
      NS_RELEASE(menuitem);
    }
    else
    {
	  nsIMenu * menu = nsnull;
	  aItem->QueryInterface(kIMenuIID, (void**) &menu);
	  if(menu)
	  {
	    // case menu
	    AddMenu(menu);
	    NS_RELEASE(menu);
	  }
	}
  }
  return NS_OK;
}

//-------------------------------------------------------------------------
NS_METHOD nsMenu::AddMenuItem(nsIMenuItem * aMenuItem)
{
  if(aMenuItem) {
    nsISupports * supports = nsnull;
    aMenuItem->QueryInterface(kISupportsIID, (void**)&supports);
    if(supports) {
	  mMenuItemVoidArray.AppendElement(supports);
      
	  nsString label;
	  aMenuItem->GetLabel(label);
	  mNumMenuItems++;
	  Str255 tmp = "\pa";
	  ::InsertMenuItem(mMacMenuHandle, tmp, mNumMenuItems);
	  NSStringSetMenuItemText(mMacMenuHandle, mNumMenuItems,label);
	}
  }
  return NS_OK;
}

//-------------------------------------------------------------------------
NS_METHOD nsMenu::AddMenu(nsIMenu * aMenu)
{
  // Add a submenu
  if(aMenu) {
    nsISupports * supports = nsnull;
    aMenu->QueryInterface(kISupportsIID, (void**)&supports);
    if(supports) {
      mMenuItemVoidArray.AppendElement(supports);
  
      // We have to add it as a menu item and then associate it with the item
      nsString label;
      aMenu->GetLabel(label);
      mNumMenuItems++;

      ::InsertMenuItem(mMacMenuHandle, "\p ", mNumMenuItems);
      NSStringSetMenuItemText(mMacMenuHandle, mNumMenuItems, label);
  
      MenuHandle menuHandle;
      aMenu->GetNativeData((void**)&menuHandle);
      ::InsertMenu(menuHandle, hierMenu);
      PRInt16 temp = mMacMenuIDCount;
      ::SetMenuItemHierarchicalID((MenuHandle) mMacMenuHandle, mNumMenuItems, --temp);
    }
  }
  return NS_OK;
}

//-------------------------------------------------------------------------
NS_METHOD nsMenu::AddSeparator() 
{
  // HACK - We're not really appending an nsMenuItem but it 
  // needs to be here to make sure that event dispatching isn't off by one.
  mMenuItemVoidArray.AppendElement(nsnull);
  ::InsertMenuItem(mMacMenuHandle, "\p(-", mNumMenuItems );
  mNumMenuItems++;
  return NS_OK;
}

//-------------------------------------------------------------------------
NS_METHOD nsMenu::GetItemCount(PRUint32 &aCount)
{
  return NS_OK;
}

//-------------------------------------------------------------------------
NS_METHOD nsMenu::GetItemAt(const PRUint32 aPos, nsISupports *& aMenuItem)
{
  return NS_OK;
}

//-------------------------------------------------------------------------
NS_METHOD nsMenu::InsertItemAt(const PRUint32 aPos, nsISupports * aMenuItem)
{
  return NS_OK;
}

//-------------------------------------------------------------------------
NS_METHOD nsMenu::RemoveItem(const PRUint32 aPos)
{
  return NS_OK;
}

//-------------------------------------------------------------------------
NS_METHOD nsMenu::RemoveAll()
{
  return NS_OK;
}

//-------------------------------------------------------------------------
NS_METHOD nsMenu::GetNativeData(void ** aData)
{
  *aData = mMacMenuHandle;
  return NS_OK;
}

//-------------------------------------------------------------------------
NS_METHOD nsMenu::AddMenuListener(nsIMenuListener * aMenuListener)
{
  mListener = aMenuListener;
  //NS_ADDREF(mListener);
  return NS_OK;
}

//-------------------------------------------------------------------------
NS_METHOD nsMenu::RemoveMenuListener(nsIMenuListener * aMenuListener)
{
  if (aMenuListener == mListener) {
    //NS_IF_RELEASE(mListener);
  }
  return NS_OK;
}


//-------------------------------------------------------------------------
//
// nsIMenuListener interface
//
//-------------------------------------------------------------------------
nsEventStatus nsMenu::MenuItemSelected(const nsMenuEvent & aMenuEvent)
{
  nsEventStatus eventStatus = nsEventStatus_eIgnore;
      
  // Determine if this is the correct menu to handle the event
  PRInt16 menuID = HiWord(((nsMenuEvent)aMenuEvent).mCommand);
  if(mMacMenuID == menuID)
  {
    // Call MenuSelected on the correct nsMenuItem
    PRInt16 menuItemID = LoWord(((nsMenuEvent)aMenuEvent).mCommand);
    nsIMenuListener * menuListener = nsnull;
    ((nsIMenuItem*)mMenuItemVoidArray[menuItemID-1])->QueryInterface(kIMenuListenerIID, &menuListener);
	if(menuListener) {
	  eventStatus = menuListener->MenuSelected(aMenuEvent);
	  NS_IF_RELEASE(menuListener);
	}
  } 
  else
  {
    // Make sure none of our submenus are the ones that should be handling this
      for (int i = mMenuItemVoidArray.Count(); i > 0; i--)
	  {
	    if(nsnull != mMenuItemVoidArray[i-1])
	    {
		    nsIMenu * submenu = nsnull;
		    ((nsISupports*)mMenuItemVoidArray[i-1])->QueryInterface(kIMenuIID, &submenu);
		    if(submenu)
		    {
			    nsIMenuListener * menuListener = nsnull;
			    ((nsISupports*)mMenuItemVoidArray[i-1])->QueryInterface(kIMenuListenerIID, &menuListener);
			    if(menuListener){
			      eventStatus = menuListener->MenuSelected(aMenuEvent);
			      NS_IF_RELEASE(menuListener);
			      if(nsEventStatus_eIgnore != eventStatus)
			        return eventStatus;
			    }
		    }
		}
	  }
  
  }
  return eventStatus;
}

nsEventStatus nsMenu::MenuSelected(const nsMenuEvent & aMenuEvent)
{
  nsEventStatus eventStatus = nsEventStatus_eIgnore;
      
  // Determine if this is the correct menu to handle the event
  PRInt16 menuID = HiWord(((nsMenuEvent)aMenuEvent).mCommand);

#ifdef APPLE_MENU_HACK
  if(kAppleMenuID == menuID)
	{
    PRInt16 menuItemID = LoWord(((nsMenuEvent)aMenuEvent).mCommand);

		if (menuItemID > 2)			// don't handle the about or separator items yet
		{
			Str255		itemStr;
			::GetMenuItemText(GetMenuHandle(menuID), menuItemID, itemStr);
#if !TARGET_CARBON
			::OpenDeskAcc(itemStr);
#endif
			eventStatus = nsEventStatus_eConsumeNoDefault;
		}
	}
	else
#endif
  if(mMacMenuID == menuID)
  {
    // Call MenuSelected on the correct nsMenuItem
    PRInt16 menuItemID = LoWord(((nsMenuEvent)aMenuEvent).mCommand);
    nsIMenuListener * menuListener = nsnull;
    ((nsIMenuItem*)mMenuItemVoidArray[menuItemID-1])->QueryInterface(kIMenuListenerIID, &menuListener);
	if(menuListener) {
	  eventStatus = menuListener->MenuSelected(aMenuEvent);
	  NS_IF_RELEASE(menuListener);
	}
  } 
  else
  {
    // Make sure none of our submenus are the ones that should be handling this
      for (int i = mMenuItemVoidArray.Count(); i > 0; i--)
	  {
	    if(nsnull != mMenuItemVoidArray[i-1])
	    {
		    nsIMenu * submenu = nsnull;
		    ((nsISupports*)mMenuItemVoidArray[i-1])->QueryInterface(kIMenuIID, &submenu);
		    if(submenu)
		    {
			    nsIMenuListener * menuListener = nsnull;
			    ((nsISupports*)mMenuItemVoidArray[i-1])->QueryInterface(kIMenuListenerIID, &menuListener);
			    if(menuListener){
			      eventStatus = menuListener->MenuSelected(aMenuEvent);
			      NS_IF_RELEASE(menuListener);
			      if(nsEventStatus_eIgnore != eventStatus)
			        return eventStatus;
			    }
		    }
		}
	  }
  
  }
  return eventStatus;
}

//-------------------------------------------------------------------------
nsEventStatus nsMenu::MenuDeselected(const nsMenuEvent & aMenuEvent)
{
  return nsEventStatus_eIgnore;
}

//-------------------------------------------------------------------------
nsEventStatus nsMenu::MenuConstruct(
    const nsMenuEvent & aMenuEvent,
    nsIWidget         * aParentWindow, 
    void              * menuNode,
	void              * aWebShell)
{
  return nsEventStatus_eIgnore;
}

//-------------------------------------------------------------------------
nsEventStatus nsMenu::MenuDestruct(const nsMenuEvent & aMenuEvent)
{
  return nsEventStatus_eIgnore;
}

//-------------------------------------------------------------------------
/**
* Set DOMNode
*
*/
NS_METHOD nsMenu::SetDOMNode(nsIDOMNode * aMenuNode)
{
	return NS_OK;
}

//-------------------------------------------------------------------------
/**
* Set DOMElement
*
*/
NS_METHOD nsMenu::SetDOMElement(nsIDOMElement * aMenuElement)
{
	return NS_OK;
}
    
//-------------------------------------------------------------------------
/**
* Set WebShell
*
*/
NS_METHOD nsMenu::SetWebShell(nsIWebShell * aWebShell)
{
	return NS_OK;
}
//-------------------------------------------------------------------------
/**
* Set WebShell
*
*/
void nsMenu::NSStringSetMenuItemText(MenuHandle macMenuHandle, short menuItem, nsString& menuString)
{
	OSErr					err;
	const PRUnichar*		unicodeText;
	char*					scriptRunText;
	size_t					unicodeTextLengthInBytes, unicdeTextReadInBytes,
							scriptRunTextSizeInBytes, scriptRunTextLengthInBytes,
							scriptCodeRunListLength;
	ScriptCodeRun			convertedTextScript;
	short					themeFontID;
	Str255					themeFontName;
	SInt16					themeFontSize;
	Style					themeFontStyle;
	
	//
	// extract the Unicode text from the nsString and convert it into a single script run
	//
	unicodeText = menuString.GetUnicode();
	unicodeTextLengthInBytes = menuString.Length() * sizeof(PRUnichar);
	scriptRunTextSizeInBytes = unicodeTextLengthInBytes * 2;
	scriptRunText = new char[scriptRunTextSizeInBytes];
	
	err = ::ConvertFromUnicodeToScriptCodeRun(mUnicodeTextRunConverter,
				unicodeTextLengthInBytes,unicodeText,
				0, /* no flags*/
				0,NULL,NULL,NULL, /* no offset arrays */
				scriptRunTextSizeInBytes,&unicdeTextReadInBytes,&scriptRunTextLengthInBytes,
				scriptRunText,
				1 /* count of script runs*/,&scriptCodeRunListLength,&convertedTextScript);
	NS_ASSERTION(err==noErr,"nsMenu::NSStringSetMenuItemText: ConvertFromUnicodeToScriptCodeRun failed.");
	if (err!=noErr) { delete [] scriptRunText; return; }
	scriptRunText[scriptRunTextLengthInBytes] = 0;	// null terminate
	
	//
	// get a font from the script code
	//
	err = ::GetThemeFont(kThemeSystemFont,convertedTextScript.script,themeFontName,&themeFontSize,&themeFontStyle);
	NS_ASSERTION(err==noErr,"nsMenu::NSStringSetMenuItemText: GetThemeFont failed.");
	if (err!=noErr) { delete [] scriptRunText; return; }
	::GetFNum(themeFontName,&themeFontID);
	err = ::SetMenuItemFontID(macMenuHandle,menuItem,themeFontID);
	NS_ASSERTION(err==noErr,"nsMenu::NSStringSetMenuItemText: SetMenuItemFontID failed.");
	::SetMenuItemText(macMenuHandle,menuItem,c2pstr(scriptRunText));
	
	
	//
	// clean up and exit
	//
	delete [] scriptRunText;
			
}

MenuHandle nsMenu::NSStringNewMenu(short menuID, nsString& menuTitle)
{
	OSErr					err;
	const PRUnichar*		unicodeText;
	char*					scriptRunText;
	size_t					unicodeTextLengthInBytes, unicdeTextReadInBytes,
							scriptRunTextSizeInBytes, scriptRunTextLengthInBytes,
							scriptCodeRunListLength;
	ScriptCodeRun			convertedTextScript;
	short					themeFontID;
	Str255					themeFontName;
	SInt16					themeFontSize;
	Style					themeFontStyle;
	MenuHandle				newMenuHandle;
	
	//
	// extract the Unicode text from the nsString and convert it into a single script run
	//
	unicodeText = menuTitle.GetUnicode();
	unicodeTextLengthInBytes = menuTitle.Length() * sizeof(PRUnichar);
	scriptRunTextSizeInBytes = unicodeTextLengthInBytes * 2;
	scriptRunText = new char[scriptRunTextSizeInBytes];
	
	err = ::ConvertFromUnicodeToScriptCodeRun(mUnicodeTextRunConverter,
				unicodeTextLengthInBytes,unicodeText,
				0, /* no flags*/
				0,NULL,NULL,NULL, /* no offset arrays */
				scriptRunTextSizeInBytes,&unicdeTextReadInBytes,&scriptRunTextLengthInBytes,
				scriptRunText,
				1 /* count of script runs*/,&scriptCodeRunListLength,&convertedTextScript);
	NS_ASSERTION(err==noErr,"nsMenu::NSStringNewMenu: ConvertFromUnicodeToScriptCodeRun failed.");
	if (err!=noErr) { delete [] scriptRunText; return NULL; }
	scriptRunText[scriptRunTextLengthInBytes] = 0;	// null terminate
	
	//
	// get a font from the script code
	//
	err = ::GetThemeFont(kThemeSystemFont,convertedTextScript.script,themeFontName,&themeFontSize,&themeFontStyle);
	NS_ASSERTION(err==noErr,"nsMenu::NSStringNewMenu: GetThemeFont failed.");
	if (err!=noErr) { delete [] scriptRunText; return NULL; }
	::GetFNum(themeFontName,&themeFontID);
	newMenuHandle = ::NewMenu(menuID,c2pstr(scriptRunText));
	NS_ASSERTION(newMenuHandle!=NULL,"nsMenu::NSStringNewMenu: NewMenu failed.");
	if (newMenuHandle==NULL) { delete [] scriptRunText; return NULL; }
	err = SetMenuFont(newMenuHandle,themeFontID,themeFontSize);
	NS_ASSERTION(err==noErr,"nsMenu::NSStringNewMenu: SetMenuFont failed.");
	if (err!=noErr) { delete [] scriptRunText; return NULL; }

	//
	// clean up and exit
	//
	delete [] scriptRunText;
	return newMenuHandle;
			
}
			