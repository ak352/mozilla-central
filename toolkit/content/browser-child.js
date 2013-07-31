/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

let Cc = Components.classes;
let Ci = Components.interfaces;
let Cu = Components.utils;

Cu.import("resource://gre/modules/Services.jsm");
Cu.import('resource://gre/modules/XPCOMUtils.jsm');

let global = this;

let WebProgressListener = {
  init: function() {
    let webProgress = docShell.QueryInterface(Ci.nsIInterfaceRequestor)
                              .getInterface(Ci.nsIWebProgress);
    webProgress.addProgressListener(this, Ci.nsIWebProgress.NOTIFY_ALL);
  },

  _requestSpec: function (aRequest) {
    if (!aRequest || !(aRequest instanceof Ci.nsIChannel))
      return null;
    return aRequest.QueryInterface(Ci.nsIChannel).URI.spec;
  },

  _setupJSON: function setupJSON(aWebProgress, aRequest) {
    return {
      isTopLevel: aWebProgress.isTopLevel,
      DOMWindowID: aWebProgress.DOMWindowID,
      isLoadingDocument: aWebProgress.isLoadingDocument,
      requestURI: this._requestSpec(aRequest),
      loadType: aWebProgress.loadType
    };
  },

  _setupObjects: function setupObjects(aWebProgress) {
    let win = docShell.QueryInterface(Ci.nsIInterfaceRequestor)
                      .getInterface(Ci.nsIDOMWindow);
    return {
      contentWindow: win,
      // DOMWindow is not necessarily the content-window with subframes.
      DOMWindow: aWebProgress.DOMWindow
    };
  },

  onStateChange: function onStateChange(aWebProgress, aRequest, aStateFlags, aStatus) {
    let json = this._setupJSON(aWebProgress, aRequest);
    let objects = this._setupObjects(aWebProgress);

    json.stateFlags = aStateFlags;
    json.status = aStatus;

    sendAsyncMessage("Content:StateChange", json, objects);
  },

  onProgressChange: function onProgressChange(aWebProgress, aRequest, aCurSelf, aMaxSelf, aCurTotal, aMaxTotal) {
  },

  onLocationChange: function onLocationChange(aWebProgress, aRequest, aLocationURI, aFlags) {
    let spec = aLocationURI ? aLocationURI.spec : "";
    let charset = content.document.characterSet;

    let json = this._setupJSON(aWebProgress, aRequest);
    let objects = this._setupObjects(aWebProgress);

    json.documentURI = aWebProgress.DOMWindow.document.documentURIObject.spec;
    json.location = spec;
    json.canGoBack = docShell.canGoBack;
    json.canGoForward = docShell.canGoForward;
    json.charset = charset.toString();

    sendAsyncMessage("Content:LocationChange", json, objects);
  },

  onStatusChange: function onStatusChange(aWebProgress, aRequest, aStatus, aMessage) {
    let json = this._setupJSON(aWebProgress, aRequest);
    let objects = this._setupObjects(aWebProgress);

    json.status = aStatus;
    json.message = aMessage;

    sendAsyncMessage("Content:StatusChange", json, objects);
  },

  onSecurityChange: function onSecurityChange(aWebProgress, aRequest, aState) {
    let json = this._setupJSON(aWebProgress, aRequest);
    let objects = this._setupObjects(aWebProgress);

    json.state = aState;
    json.status = SecurityUI.getSSLStatusAsString();

    sendAsyncMessage("Content:SecurityChange", json, objects);
  },

  QueryInterface: function QueryInterface(aIID) {
    if (aIID.equals(Ci.nsIWebProgressListener) ||
        aIID.equals(Ci.nsISupportsWeakReference) ||
        aIID.equals(Ci.nsISupports)) {
        return this;
    }

    throw Components.results.NS_ERROR_NO_INTERFACE;
  }
};

WebProgressListener.init();

let WebNavigation =  {
  _webNavigation: docShell.QueryInterface(Ci.nsIWebNavigation),

  init: function() {
    addMessageListener("WebNavigation:GoBack", this);
    addMessageListener("WebNavigation:GoForward", this);
    addMessageListener("WebNavigation:GotoIndex", this);
    addMessageListener("WebNavigation:LoadURI", this);
    addMessageListener("WebNavigation:Reload", this);
    addMessageListener("WebNavigation:Stop", this);
  },

  receiveMessage: function(message) {
    switch (message.name) {
      case "WebNavigation:GoBack":
        this.goBack();
        break;
      case "WebNavigation:GoForward":
        this.goForward();
        break;
      case "WebNavigation:GotoIndex":
        this.gotoIndex(message);
        break;
      case "WebNavigation:LoadURI":
        this.loadURI(message);
        break;
      case "WebNavigation:Reload":
        this.reload(message);
        break;
      case "WebNavigation:Stop":
        this.stop(message);
        break;
    }
  },

  goBack: function() {
    if (this._webNavigation.canGoBack)
      this._webNavigation.goBack();
  },

  goForward: function() {
    if (this._webNavigation.canGoForward)
      this._webNavigation.goForward();
  },

  gotoIndex: function(message) {
    this._webNavigation.gotoIndex(message.index);
  },

  loadURI: function(message) {
    let flags = message.json.flags || this._webNavigation.LOAD_FLAGS_NONE;
    this._webNavigation.loadURI(message.json.uri, flags, null, null, null);
  },

  reload: function(message) {
    let flags = message.json.flags || this._webNavigation.LOAD_FLAGS_NONE;
    this._webNavigation.reload(flags);
  },

  stop: function(message) {
    let flags = message.json.flags || this._webNavigation.STOP_ALL;
    this._webNavigation.stop(flags);
  }
};

WebNavigation.init();

let SecurityUI = {
  getSSLStatusAsString: function() {
    let status = docShell.securityUI.QueryInterface(Ci.nsISSLStatusProvider).SSLStatus;

    if (status) {
      let helper = Cc["@mozilla.org/network/serialization-helper;1"]
                      .getService(Ci.nsISerializationHelper);

      status.QueryInterface(Ci.nsISerializable);
      return helper.serializeToString(status);
    }

    return null;
  }
}

let Content = {
  init: function init() {
    docShell.useGlobalHistory = true;

    Cc["@mozilla.org/eventlistenerservice;1"]
      .getService(Ci.nsIEventListenerService)
      .addSystemEventListener(global, "click", this.contentAreaClick.bind(this), true);

    addMessageListener("StyleSheet:Load", this);
    addMessageListener("Content:DoCommand", this);
    addMessageListener("Content:FullZoom", this);
    addMessageListener("Content:TextZoom", this);
  },

  receiveMessage: function(aMessage) {
    let json = aMessage.json;
    switch (aMessage.name) {
    case "StyleSheet:Load":
      let uri = Services.io.newURI(json.href, null, null);
      let styleSheets = Cc["@mozilla.org/content/style-sheet-service;1"].getService(Ci.nsIStyleSheetService);
      if (!styleSheets.sheetRegistered(uri, Ci.nsIStyleSheetService.USER_SHEET))
	      styleSheets.loadAndRegisterSheet(uri, Ci.nsIStyleSheetService.USER_SHEET);
      break;
    case "Content:DoCommand":
      let command = json.command;
      try {
        docShell.doCommand(command);
      }
      catch (e) {
        Components.utils.reportError("An error occurred executing the " +
                                     command + " command: " + e);
        Components.utils.reportError((new Error).stack);
      }
      break;
    case "Content:FullZoom": {
      let viewer = docShell.contentViewer.QueryInterface(Components.interfaces.nsIMarkupDocumentViewer);
      viewer.fullZoom = json.value;
     } break;
    case "Content:TextZoom": {
      let viewer = docShell.contentViewer.QueryInterface(Components.interfaces.nsIMarkupDocumentViewer);
      viewer.textZoom = json.value;
     } break;

    }
  },

  contentAreaClick: function(event) {
    if (!event.isTrusted || event.defaultPrevented || event.button == 2)
      return;

    let [href, node] = this._hrefAndLinkNodeForClickEvent(event);

    let json = { button: event.button, shiftKey: event.shiftKey,
                 ctrlKey: event.ctrlKey, metaKey: event.metaKey,
                 altKey: event.altKey, href: null, title: null,
                 bookmark: false };

    if (href) {
      json.href = href;
      if (node) {
        json.title = node.getAttribute("title");

        if (event.button == 0 && !event.ctrlKey && !event.shiftKey &&
            !event.altKey && !event.metaKey) {
          json.bookmark = node.getAttribute("rel") == "sidebar";
          if (json.bookmark)
            event.preventDefault(); // Need to prevent the pageload.
        }
      }

      sendAsyncMessage("Content:Click", json);
      return;
    }

    // This might be middle mouse navigation.
    if (event.button == 1)
      sendAsyncMessage("Content:Click", json);
  },

/**
 * Extracts linkNode and href for the current click target.
 *
 * @param event
 *        The click event.
 * @return [href, linkNode].
 *
 * @note linkNode will be null if the click wasn't on an anchor
 *       element (or XLink).
 */
  _hrefAndLinkNodeForClickEvent: function(event) {
    function isHTMLLink(aNode)
    {
      // Be consistent with what nsContextMenu.js does.
      return ((aNode instanceof content.HTMLAnchorElement && aNode.href) ||
              (aNode instanceof content.HTMLAreaElement && aNode.href) ||
              aNode instanceof content.HTMLLinkElement);
    }

    function makeURLAbsolute(aBase, aUrl)
    {
      // Note:  makeURI() will throw if aUri is not a valid URI
      return makeURI(aUrl, null, makeURI(aBase)).spec;
    }

    let node = event.target;
    while (node && !isHTMLLink(node)) {
      node = node.parentNode;
    }

    if (node)
      return [node.href, node];

    // If there is no linkNode, try simple XLink.
    let href, baseURI;
    node = event.target;
    while (node && !href) {
      if (node.nodeType == content.Node.ELEMENT_NODE) {
        href = node.getAttributeNS("http://www.w3.org/1999/xlink", "href");
        if (href)
          baseURI = node.baseURI;
      }
      node = node.parentNode;
    }

    // In case of XLink, we don't return the node we got href from since
    // callers expect <a>-like elements.
    return [href ? makeURLAbsolute(baseURI, href) : null, null];
  }
};

Content.init();

let AddonListeners = {
  classDescription: "Addon shim content policy",
  classID: Components.ID("6e869130-635c-11e2-bcfd-0800200c9a66"),
  contractID: "@mozilla.org/addonjunk/policy;1",
  xpcom_categories: ["content-policy"],

  init: function init() {
    Services.obs.addObserver(this, "content-document-global-created", false);

    let registrar = Components.manager.QueryInterface(Ci.nsIComponentRegistrar);
    registrar.registerFactory(this.classID, this.classDescription, this.contractID, this);

    //let xpcom_categories = ["content-policy", "net-channel-event-sinks"];
    let xpcom_categories = ["content-policy"];
    var catMan = Cc["@mozilla.org/categorymanager;1"].getService(Ci.nsICategoryManager);
    for each (let category in this.xpcom_categories)
      catMan.addCategoryEntry(category, this.contractID, this.contractID, false, true);
  },

  QueryInterface: XPCOMUtils.generateQI([Ci.nsIContentPolicy, Ci.nsIObserver,
                                         Ci.nsIChannelEventSink, Ci.nsIFactory,
                                         Ci.nsISupportsWeakReference]),

  observe: function AddonListeners_observe(aSubject, aTopic, aData) {
    sendSyncMessage("Addon:Observe", {
      topic: aTopic,
      data: aData
    });
  },

  shouldLoad: function(contentType, contentLocation, requestOrigin, node, mimeTypeGuess, extra) {
    var rval = sendSyncMessage("Addon:ShouldLoad", {
      contentType: contentType,
      mimeTypeGuess: mimeTypeGuess
    }, {
      contentLocation: contentLocation,
      requestOrigin: requestOrigin,
      node: node
    });
    if (rval == undefined)
      return Ci.nsIContentPolicy.ACCEPT;
    return rval;
  },

  shouldProcess: function(contentType, contentLocation, requestOrigin, insecNode, mimeType, extra) {
    return Ci.nsIContentPolicy.ACCEPT;
  },

  createInstance: function(outer, iid) {
    if (outer)
      throw Cr.NS_ERROR_NO_AGGREGATION;
    return this.QueryInterface(iid);
  }
};

AddonListeners.init();

let FinderListener = {
  init: function init() {
    let Finder = Components.utils.import("resource://gre/modules/Finder.jsm", {}).Finder;
    this.finder = new Finder(docShell);
    this.finder.addResultListener(this);

    addMessageListener("Finder:CaseSensitive", this);
    addMessageListener("Finder:FastFind", this);
    addMessageListener("Finder:FindAgain", this);
    addMessageListener("Finder:Highlight", this);
    addMessageListener("Finder:RemoveSelection", this);
    addMessageListener("Finder:FocusContent", this);
    addMessageListener("Finder:KeyPress", this);
  },

  onFindResult: function (aResult, aFindBackwards) {
    sendSyncMessage("Finder:Result", {result: aResult, findBackwards: aFindBackwards,
      searchString: this.finder.searchString});
  },

  receiveMessage: function(aMessage) {
    let json = aMessage.json;
    dump("receiveMessage: " + aMessage.name + "\n");
    switch (aMessage.name) {
    case "Finder:CaseSensitive":
      this.finder.caseSensitive = json.caseSensitive;
      break;
    case "Finder:FastFind":
      this.finder.fastFind(json.searchString, json.linksOnly);
      break;
    case "Finder:FindAgain":
      this.finder.findAgain(json.findBackwards, json.linksOnly);
      break;
    case "Finder:Highlight":
      this.finder.highlight(json.highlight, json.word);
      break;
    case "Finder:RemoveSelection":
      this.finder.removeSelection();
      break;
    case "Finder:FocusContent":
      this.finder.focusContent();
      break;
    case "Finder:KeyPress":
      this.finder.keyPress(json);
      break;
    }
  }
}

FinderListener.init();

addEventListener("DOMTitleChanged", function (aEvent) {
  let document = content.document;
  switch (aEvent.type) {
  case "DOMTitleChanged":
    if (!aEvent.isTrusted || aEvent.target.defaultView != content)
      return;

    sendAsyncMessage("DOMTitleChanged", { title: document.title });
    break;
  }
}, false);

addEventListener("ImageContentLoaded", function (aEvent) {
  if (content.document instanceof Ci.nsIImageDocument) {
    let req = content.document.imageRequest;
    if (!req.image)
      return;
    sendAsyncMessage("ImageDocumentLoaded", { width: req.image.width,
                                              height: req.image.height });
  }
}, false)
