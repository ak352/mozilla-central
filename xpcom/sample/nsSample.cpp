/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
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


/*

  A sample of XPConnect. This file contains an implementation of
  nsISample.

*/
#include "nscore.h"
#include "nsISample.h"
#include "nsIAllocator.h"
#include "plstr.h"
#include "stdio.h"

/**
 * SampleImpl is an implementation of the nsISample interface.  In XPCOM,
 * there can be more than one implementation of an given interface.  Class
 * IDs (CIDs) uniquely identify a particular implementation of an interface.
 * Interface IDs (IIDs) uniquely identify an interface.
 */
class SampleImpl : public nsISample
{
public:
    SampleImpl();
    virtual ~SampleImpl();

    /**
     * This macro expands into a declaration of the nsISupports interface.
     * Every XPCOM component needs to implement nsISupports, as it acts
     * as the gateway to other interfaces this component implements.  You
     * could manually declare QueryInterface, AddRef, and Release instead
     * of using this macro, but why?
     */
    // nsISupports interface
    NS_DECL_ISUPPORTS

    /**
     * This macro is defined in the nsISample.h file, and is generated
     * automatically by the xpidl compiler.  It expands to
     * declarations of all of the methods required to implement the
     * interface.  xpidl will generate a NS_DECL_[INTERFACENAME] macro
     * for each interface that it processes.
     *
     * The methods of nsISample are discussed individually below, but
     * commented out (because this macro already defines them.)
     */
    NS_DECL_NSISAMPLE

    /**
     * NS_IMETHOD expands to the standard XPCOM return type.  XPCOM methods
     * should never return any other type.  The return value is used
     * behind the scenes by the XPConnect runtime to figure out if the call
     * failed in any way.
     * These methods were generated by "attribute string Value" in 
     * nsISample.idl.  When reflected into JavaScript, XPCOM will use these
     * calls as Getter/Setter ops, so that they can be called transparently
     * as "sample.Value='foo';" and "var val = sample.Value"
     */
    // nsISample interface
    /* NS_IMETHOD GetValue(char * *aValue); */
    /* NS_IMETHOD SetValue(char * aValue); */

    /**
     * The const came from the "in" specifier in nsISample.idl.  "in"
     * specifies that the value of this parameter is used only for input,
     * this method is not allowed to modify the contents of the buffer.
     */
    /* NS_IMETHOD WriteValue(const char *aPrefix); */

    /**
     * nsISample.idl specifies all of it's string types as string, instead
     * of wstring (wide string), the Unicode type.  If the world were a
     * perfect place, all normal strings in XPCOM interfaces would be unicode.
     * If this type had been specified as wstring, it would appear as
     * PRUnichar * in C++, which is the NSPR type for unicode characters.
     */
    /* NS_IMETHOD Poke(const char* aValue); */

private:
    char* mValue;
};

////////////////////////////////////////////////////////////////////////

/**
 * This is the static constructor for the sample component.  Notice that
 * the prototype for this function is included in the {C++ ... } section
 * of nsISample.idl.  This prototype is not actually part of the nsISample
 * interface, it only gets included, verbatim, in nsISample.h.
 * This is so that the factory for this class (nsSampleFactory.cpp)
 * can create a nsSample object.  Normally you would expect to use
 * "SampleImpl s = new SampleImpl();" to create the object, the catch here
 * is that SampleImpl is not declared anywhere except in this file, so the
 * factory has no idea what a SampleImpl is.  Instead, this static function's
 * prototype is declared in in nsISample.h (generated from nsISample.idl),
 * which any nsISample factory would require for the declaration of
 * nsISample anyway.
 */
nsresult
NS_NewSample(nsISample** aSample)
{
    NS_PRECONDITION(aSample != nsnull, "null ptr");
    if (! aSample)
        return NS_ERROR_NULL_POINTER;

    *aSample = new SampleImpl();
    if (! *aSample)
        return NS_ERROR_OUT_OF_MEMORY;

    /**
     * XPCOM automatically frees up memory used by objects when they are
     * no longer in use.  It determines that an object is no longer in use
     * by checking how many unique, owning references there are to it.
     * Unfortunately, there is no automatic procedure for determining
     * what an owning reference is.  Ownership is determined by conventions,
     * and you must be very careful to adhere to these conventions, or you
     * will forever be plagued by circular dependancies, and memory leaks.
     * The first rule of ownership is, "If You Created It, You Own It"
     * The other part of this convention is, when you create a new
     * object, the factory has already added you as an owning reference.
     * It is the clients responsibility to call Release() when it is finished
     * using the object.
     * NS_ADDREF() takes care of calling AddRef on the nsISupports interface
     * of the object you pass it.
     */
    NS_ADDREF(*aSample);

    return NS_OK;
}

////////////////////////////////////////////////////////////////////////

SampleImpl::SampleImpl() : mValue(nsnull)
{
    NS_INIT_REFCNT();
    mValue = PL_strdup("initial value");
}

SampleImpl::~SampleImpl()
{
    if (mValue)
        PL_strfree(mValue);
}

/**
 * NS_IMPL_ISUPPORTS expands to a simple implementation of the nsISupports
 * interface.  This includes a proper implementation of AddRef, Release,
 * and QueryInterface.  If this class supported more interfaces than just
 * nsISupports, 
 * you could use NS_IMPL_ADDREF() and NS_IMPL_RELEASE() to take care of the
 * simple stuff, but you would have to create QueryInterface on your own.
 * nsSampleFactory.cpp is an example of this approach.
 * Notice that the second parameter to the macro is the static IID accessor
 * method, and NOT the #defined IID.
 */
NS_IMPL_ISUPPORTS(SampleImpl, nsISample::GetIID());

/**
 * Notice that in the protoype for this function, the NS_IMETHOD macro was
 * used to declare the return type.  For the implementation, the return
 * type is declared by NS_IMETHODIMP
 */
NS_IMETHODIMP
SampleImpl::GetValue(char** aValue)
{
    NS_PRECONDITION(aValue != nsnull, "null ptr");
    if (! aValue)
        return NS_ERROR_NULL_POINTER;

    if (mValue) {
        /**
         * GetValue's job is to return data known by an instance of
         * SampleImpl to the outside world.  If we  were to simply return 
         * a pointer to data owned by this instance, and the client were to
         * free it, bad things would surely follow.
         * On the other hand, if we create a new copy of the data for our
         * client, and it turns out that client is implemented in JavaScript,
         * there would be no way to free the buffer.  The solution to the 
         * buffer ownership problem is the nsAllocator singleton.  Any buffer
         * returned by an XPCOM method should be allocated by the nsAllocator.
         * This convention lets things like JavaScript reflection do their
         * job, and simplifies the way C++ clients deal with returned buffers.
         */
        *aValue = (char*) nsAllocator::Alloc(PL_strlen(mValue) + 1);
        if (! *aValue)
            return NS_ERROR_NULL_POINTER;

        PL_strcpy(*aValue, mValue);
    }
    else {
        *aValue = nsnull;
    }
    return NS_OK;
}

NS_IMETHODIMP
SampleImpl::SetValue(const char* aValue)
{
    NS_PRECONDITION(aValue != nsnull, "null ptr");
    if (! aValue)
        return NS_ERROR_NULL_POINTER;

    if (mValue) {
        PL_strfree(mValue);
    }

    /**
     * Another buffer passing convention is that buffers passed INTO your
     * object ARE NOT YOURS.  Keep your hands off them, unless they are
     * declared "inout".  If you want to keep the value for posterity,
     * you will have to make a copy of it.
     */
    mValue = PL_strdup(aValue);
    return NS_OK;
}

NS_IMETHODIMP
SampleImpl::Poke(const char* aValue)
{
    return SetValue((char*) aValue);
}

NS_IMETHODIMP
SampleImpl::WriteValue(const char* aPrefix)
{
    NS_PRECONDITION(aPrefix != nsnull, "null ptr");
    if (! aPrefix)
        return NS_ERROR_NULL_POINTER;

    printf("%s %s\n", aPrefix, mValue);
    return NS_OK;
}
