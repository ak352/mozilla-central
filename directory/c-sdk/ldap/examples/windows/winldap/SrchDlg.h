/* ***** BEGIN LICENSE BLOCK *****
 * Version: NPL 1.1/GPL 2.0/LGPL 2.1
 * 
 * The contents of this file are subject to the Netscape Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/NPL/
 * 
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 * 
 * The Original Code is Mozilla Communicator client code.
 * 
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1996-1999
 * the Initial Developer. All Rights Reserved.
 * 
 * Contributor(s):
 * 
 * Alternatively, the contents of this file may be used under the terms of
 * either of the GNU General Public License Version 2 or later (the "GPL"),
 * or the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the NPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the NPL, the GPL or the LGPL.
 * 
 * ***** END LICENSE BLOCK ***** */

// SrchDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// SearchDlg dialog

class SearchDlg : public CDialog
{
// Construction
public:
	SearchDlg(CWnd* pParent = NULL);   // standard constructor

public:
	void SetScope( int scope ) { m_scope = scope; }
	int GetScope() { return m_scope; }

private:
	int m_scope;

public:
// Dialog Data
	//{{AFX_DATA(SearchDlg)
	enum { IDD = IDD_SEARCH_DIALOG };
	CString	m_searchBase;
	CString	m_searchFilter;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(SearchDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(SearchDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnScopeBase();
	afx_msg void OnScopeOne();
	afx_msg void OnScopeSub();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
