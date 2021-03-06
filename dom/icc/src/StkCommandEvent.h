/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_StkCommandEvent_h
#define mozilla_dom_StkCommandEvent_h

#include "mozilla/dom/MozStkCommandEventBinding.h"
#include "nsDOMEvent.h"
#include "SimToolKit.h"

namespace mozilla {
namespace dom {

class StkCommandEvent : public nsDOMEvent,
                        public nsIDOMMozStkCommandEvent
{
  nsString mCommand;

public:
  NS_DECL_ISUPPORTS_INHERITED
  NS_FORWARD_TO_NSDOMEVENT
  NS_DECL_NSIDOMMOZSTKCOMMANDEVENT

  static already_AddRefed<StkCommandEvent>
  Create(EventTarget* aOwner, const nsAString& aMessage);

  nsresult
  Dispatch(EventTarget* aTarget, const nsAString& aEventType)
  {
    NS_ASSERTION(aTarget, "Null pointer!");
    NS_ASSERTION(!aEventType.IsEmpty(), "Empty event type!");

    nsresult rv = InitEvent(aEventType, false, false);
    NS_ENSURE_SUCCESS(rv, rv);

    SetTrusted(true);

    nsDOMEvent* thisEvent = this;

    bool dummy;
    rv = aTarget->DispatchEvent(thisEvent, &dummy);
    NS_ENSURE_SUCCESS(rv, rv);

    return NS_OK;
  }

  virtual JSObject* WrapObject(JSContext* aCx,
                               JS::Handle<JSObject*> aScope) MOZ_OVERRIDE
  {
    return MozStkCommandEventBinding::Wrap(aCx, aScope, this);
  }

  JS::Value GetCommand(JSContext* aCx, ErrorResult& aRv)
  {
    JS::Rooted<JS::Value> retVal(aCx);
    aRv = GetCommand(aCx, retVal.address());
    return retVal;
  }

private:
  StkCommandEvent(EventTarget* aOwner)
  : nsDOMEvent(aOwner, nullptr, nullptr)
  {
    SetIsDOMBinding();
  }

  ~StkCommandEvent()
  { }
};

} // namespace dom
} // namespace mozilla

#endif // mozilla_dom_StkCommandEvent_h
