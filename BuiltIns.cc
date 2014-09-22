// Copyright (C) 2013 The University of Michigan
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Authors - Chun-Hung Hsiao (chhsiao@umich.edu)
//

#include "BuiltIns.h"

BuiltIns BuiltIns::builtins_;

BuiltIns::BuiltIns() {
    // JS Global functions
    functions_.insert("decodeURI");
    functions_.insert("decodeURIComponent");
    functions_.insert("encodeURI");
    functions_.insert("encodeURIComponent");
    functions_.insert("escape");
    functions_.insert("eval");
    functions_.insert("isFinite");
    functions_.insert("isNaN");
    functions_.insert("Number");
    functions_.insert("parseFloat");
    functions_.insert("parseInt");
    functions_.insert("String");
    functions_.insert("unescape");
    // JS Array methods
    methods_.insert("concat");
    methods_.insert("indexOf");
    methods_.insert("join");
    methods_.insert("lastIndexOf");
    methods_.insert("pop");
    methods_.insert("push");
    methods_.insert("reverse");
    methods_.insert("shift");
    methods_.insert("slice");
    methods_.insert("sort");
    methods_.insert("splice");
    methods_.insert("toString");
    methods_.insert("unshift");
    methods_.insert("valueOf");
    // JS Boolean methods
    methods_.insert("toString");
    methods_.insert("valueOf");
    // JS Date methods
    methods_.insert("getDate");
    methods_.insert("getDay");
    methods_.insert("getFullYear");
    methods_.insert("getHours");
    methods_.insert("getMilliseconds");
    methods_.insert("getMinutes");
    methods_.insert("getMonth");
    methods_.insert("getSeconds");
    methods_.insert("getTime");
    methods_.insert("getTimezoneOffset");
    methods_.insert("getUTCDate");
    methods_.insert("getUTCDay");
    methods_.insert("getUTCFullYear");
    methods_.insert("getUTCHours");
    methods_.insert("getUTCMilliseconds");
    methods_.insert("getUTCMinutes");
    methods_.insert("getUTCMonth");
    methods_.insert("getUTCSeconds");
    methods_.insert("getYear");
    methods_.insert("parse");
    methods_.insert("setDate");
    methods_.insert("setFullYear");
    methods_.insert("setHours");
    methods_.insert("setMilliseconds");
    methods_.insert("setMinutes");
    methods_.insert("setMonth");
    methods_.insert("setSeconds");
    methods_.insert("setTime");
    methods_.insert("setUTCDate");
    methods_.insert("setUTCFullYear");
    methods_.insert("setUTCHours");
    methods_.insert("setUTCMilliseconds");
    methods_.insert("setUTCMinutes");
    methods_.insert("setUTCMonth");
    methods_.insert("setUTCSeconds");
    methods_.insert("setYear");
    methods_.insert("toDateString");
    methods_.insert("toGMTString");
    methods_.insert("toISOString");
    methods_.insert("toJSON");
    methods_.insert("toLocaleDateString");
    methods_.insert("toLocaleTimeString");
    methods_.insert("toLocaleString");
    methods_.insert("toString");
    methods_.insert("toTimeString");
    methods_.insert("toUTCString");
    methods_.insert("UTC");
    methods_.insert("valueOf");
    // JS Math methods
    methods_.insert("abs");
    methods_.insert("acos");
    methods_.insert("asin");
    methods_.insert("atan");
    methods_.insert("atan2");
    methods_.insert("ceil");
    methods_.insert("cos");
    methods_.insert("exp");
    methods_.insert("floor");
    methods_.insert("log");
    methods_.insert("max");
    methods_.insert("min");
    methods_.insert("pow");
    methods_.insert("random");
    methods_.insert("round");
    methods_.insert("sin");
    methods_.insert("sqrt");
    methods_.insert("tan");
    // JS Number methods
    methods_.insert("toExponential");
    methods_.insert("toFixed");
    methods_.insert("toPrecision");
    methods_.insert("toString");
    methods_.insert("valueOf");
    // JS String methods
    methods_.insert("charAt");
    methods_.insert("charCodeAt");
    methods_.insert("concat");
    methods_.insert("fromCharCode");
    methods_.insert("indexOf");
    methods_.insert("lastIndexOf");
    methods_.insert("match");
    methods_.insert("replace");
    methods_.insert("search");
    methods_.insert("slice");
    methods_.insert("split");
    methods_.insert("substr");
    methods_.insert("substring");
    methods_.insert("toLowerCase");
    methods_.insert("toUpperCase");
    methods_.insert("valueOf");
    methods_.insert("anchor");
    methods_.insert("big");
    methods_.insert("blink");
    methods_.insert("bold");
    methods_.insert("fixed");
    methods_.insert("fontcolor");
    methods_.insert("fontsize");
    methods_.insert("italics");
    methods_.insert("link");
    methods_.insert("small");
    methods_.insert("strike");
    methods_.insert("sub");
    methods_.insert("sup");
    // JS RegExp methods
    methods_.insert("compile");
    methods_.insert("exec");
    methods_.insert("test");
    // Window methods
    methods_.insert("alert");
    methods_.insert("blur");
    methods_.insert("clearInterval");
    methods_.insert("clearTimeout");
    methods_.insert("close");
    methods_.insert("confirm");
    methods_.insert("createPopup");
    methods_.insert("focus");
    methods_.insert("moveBy");
    methods_.insert("moveTo");
    methods_.insert("open");
    methods_.insert("print");
    methods_.insert("prompt");
    methods_.insert("resizeBy");
    methods_.insert("resizeTo");
    methods_.insert("scroll");
    methods_.insert("scrollBy");
    methods_.insert("scrollTo");
    methods_.insert("setInterval");
    methods_.insert("setTimeout");
    // Navigator methods
    methods_.insert("javaEnabled");
    methods_.insert("taintEnabled");
    // History methods
    methods_.insert("back");
    methods_.insert("forward");
    methods_.insert("go");
    // Location methods
    methods_.insert("assign");
    methods_.insert("reload");
    methods_.insert("replace");
    // DOM Node methods
    methods_.insert("appendChild");
    methods_.insert("cloneNode");
    methods_.insert("compareDocumentPosition");
    methods_.insert("getFeature");
    methods_.insert("getUserData");
    methods_.insert("hasAttributes");
    methods_.insert("hasChildNodes");
    methods_.insert("insertBefore");
    methods_.insert("isDefaultNamespace");
    methods_.insert("isEqualNode");
    methods_.insert("isSameNode");
    methods_.insert("isSupported");
    methods_.insert("lookupNamespaceURI");
    methods_.insert("lookupPrefix");
    methods_.insert("normalize");
    methods_.insert("removeChild");
    methods_.insert("replaceChild");
    methods_.insert("setUserData");
    // DOM NodeList methods
    methods_.insert("item");
    // DOM NamedNodeMap methods
    methods_.insert("getNamedItem");
    methods_.insert("getNamedItemNS");
    methods_.insert("item");
    methods_.insert("removeNamedItem");
    methods_.insert("removeNamedItemNS");
    methods_.insert("setNamedItem");
    methods_.insert("setNamedItemNS");
    // DOM Document (Core) methods
    methods_.insert("adoptNode");
    methods_.insert("createAttribute");
    methods_.insert("createAttributeNS");
    methods_.insert("createCDATASection");
    methods_.insert("createComment");
    methods_.insert("createDocumentFragment");
    methods_.insert("createElement");
    methods_.insert("createElementNS");
    methods_.insert("createEntityReference");
    methods_.insert("createProcessingInstruction");
    methods_.insert("createTextNode");
    methods_.insert("getElementById");
    methods_.insert("getElementsByTagName");
    methods_.insert("getElementsByTagNameNS");
    methods_.insert("importNode");
    methods_.insert("normalizeDocument");
    methods_.insert("renameNode");
    // DOM Element methods
    methods_.insert("getAttribute");
    methods_.insert("getAttributeNS");
    methods_.insert("getAttributeNode");
    methods_.insert("getAttributeNodeNS");
    methods_.insert("getElementsByTagName");
    methods_.insert("getElementsByTagNameNS");
    methods_.insert("hasAttribute");
    methods_.insert("hasAttributeNS");
    methods_.insert("removeAttribute");
    methods_.insert("removeAttributeNS");
    methods_.insert("removeAttributeNode");
    methods_.insert("setAttribute");
    methods_.insert("setAttributeNS");
    methods_.insert("setAttributeNode");
    methods_.insert("setAttributeNodeNS");
    methods_.insert("setIdAttribute");
    methods_.insert("setIdAttributeNS");
    methods_.insert("setIdAttributeNode");
    // DOM Document methods
    methods_.insert("close");
    methods_.insert("getElementsByName");
    methods_.insert("open");
    methods_.insert("write");
    methods_.insert("writeln");
    // DOM Event methods
    methods_.insert("initEvent");
    methods_.insert("preventDefault");
    methods_.insert("stopProgagation");
    // DOM EventTarget methods
    methods_.insert("addEventListener");
    methods_.insert("dispatchEvent");
    methods_.insert("removeEventListener");
    // DOM EventListener methods
    methods_.insert("handleEvent");
    // DOM DocumentEvent methods
    methods_.insert("createEvent");
    // DOM MouseEvent methods
    methods_.insert("initMouseEvent");
    // DOM HTMLElement methods
    methods_.insert("toString");
    // DOM Form methods
    methods_.insert("reset");
    methods_.insert("submit");
    // DOM Password methods
    methods_.insert("select");
    // DOM Text methods
    methods_.insert("select");
    // DOM Select methods
    methods_.insert("add");
    methods_.insert("remove");
    // DOM Table methods
    methods_.insert("createCaption");
    methods_.insert("createTFoot");
    methods_.insert("createTHead");
    methods_.insert("deleteCaption");
    methods_.insert("deleteRow");
    methods_.insert("deleteTFoot");
    methods_.insert("deleteTHead");
    methods_.insert("insertRow");
    // DOM tr methods
    methods_.insert("deleteCell");
    methods_.insert("insertCell");
    // DOM Textarea methods
    methods_.insert("select");
    // Object constructors
    constructors_.insert("Array");
    constructors_.insert("Boolean");
    constructors_.insert("Date");
    constructors_.insert("Function");
    constructors_.insert("Image");
    constructors_.insert("Number");
    constructors_.insert("Object");
    constructors_.insert("Option");
    constructors_.insert("RegExp");
    constructors_.insert("String");
}
