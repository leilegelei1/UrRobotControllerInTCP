//>>built
define("webview/interface",["dijit/registry","dojo/Deferred","dojo/topic","webview/utils/hash"],function(p,h,u,q){function r(a){a=a.getBoundingClientRect();return 0<=a.top&&0<=a.left&&a.bottom<=(window.innerHeight||document.documentElement.clientHeight)&&a.right<=(window.innerWidth||document.documentElement.clientWidth)}function f(a){var b=a.indexOf(":"),c,d;-1===b?c=p.byId(a):(c=p.byId(a.substring(0,b)),d=a.substring(b+1));return{app:c,path:d}}function n(a){return g[a.id]&&"undefined"!==typeof l[a.id]}
function s(a,b,c){var d,e=1,f;d=a;do f=document.querySelector(".slwebview-anchor["+d+"\x3d'"+b+"'], .slwebview-anchor["+d+"\x3d'"+c+"']"),"boolean"!==typeof m[d]&&(m[d]=null!==f||null!==document.querySelector(".slwebview-anchor["+d+"]")),!f&&m[d]&&(e+=1,d=a+"-"+String(e));while(null===f&&(m[d]||"undefined"===typeof m[d]));return f}function k(a){n(a)||(l[a.id]||(l[a.id]=a.on("open",function(b){var c=a.id+":"+b.sid;n(a)&&(a.isElement(b)?(b=a.id+":"+b.diagram.fullname+"/"+b.name,c=s("data-slwebview-elem-anchor",
b,c)):(b=a.id+":"+b.fullname,c=s("data-slwebview-diag-anchor",b,c)),c&&(r(c)||c.scrollIntoView(),c.classList.remove("slwebview-anchor-highlight"),top.slwebview.__tmp=c.offsetWidth,c.classList.add("slwebview-anchor-highlight")))})),g[a.id]=!0)}function t(a,b){function c(a){return a.replace(/-([a-z])/g,function(a){return a[1].toUpperCase()})}var d=a.substring(a.indexOf("-")+1),e=c(d),f=c(d+"-id");return function(){function c(a){var h=1,g=this.dataset[e],k=this.dataset[f],l=a&&a.nohash;for(a=a&&a.highlight;g;)b(g),
g=this.dataset[e+"-"+String(h+=1)];k&&(!l&&q()!==k)&&(d=!0,q(k));a&&(r(this)||this.scrollIntoView(),this.classList.remove("slwebview-anchor-highlight"),top.slwebview.__tmp=this.offsetWidth,this.classList.add("slwebview-anchor-highlight"))}var d=!1,k=document.querySelectorAll("["+a+"]"),g,l=k.length,h,m,n={};for(h=0;h<l;h+=1)g=k[h],g.addEventListener("click",c),(m=g.dataset[f])&&(n[m]=g);u.subscribe("/dojo/hashchange",function(a){if(a=n[a])d||c.call(a,{nohash:!0,highlight:!0}),d=!1})}}var l={},g={},
m={};return{open:function(a,b){var c=f(a),d=c.app,c=c.path,e="diagram"===(b&&b.type||"diagram")?d.getDiagram(c):d.getElement(c);return d.isDiagram(e)?d.open(e,b):d.open(e.diagram,b).then(function(){d.select(e,b);return d.open(e,b)})},close:function(a,b){var c=f(a),d=c.app,c=c.path,c="diagram"===(b&&b.type||"diagram")?d.getDiagram(c):d.getElement(c);return d.close(c)},select:function(a,b){var c=f(a),d=c.app,c=c.path,e="diagram"===(b&&b.type||"element")?d.getDiagram(c):d.getElement(c);return d.isDiagram(e)?
d.open(e,b).then(function(){d.select(e,b)}):d.isElement(e)?d.open(e.diagram,b).then(function(){d.select(e,b)}):(new h).reject("invalid path",!0)},unselect:function(a){f(a).app.unselect()},centerToView:function(a){a=f(a);var b=a.app,c=b.getElement(a.path);return c?b.open(c.diagram).then(function(){b.centerToView(c)}):(new h).reject("invalid path",!0)},moveToView:function(a){a=f(a);var b=a.app,c=b.getElement(a.path);return c?b.open(c.diagram).then(function(){b.moveToView(c)}):(new h).reject("invalid path")},
highlight:function(a,b){var c=f(a),d=c.app,e=d.getElement(c.path),g="stroke: "+b+"; stroke-width: 2; stroke-linejoin: miter; stroke-dasharray: none; fill: none;";return e?d.open(e.diagram).then(function(){d.highlight(e,g);d.moveToView(e)}):(new h).reject("invalid path")},unhighlight:function(a){a=f(a);var b=a.app,c=b.getElement(a.path);if(b.isElement(c))b.unhighlight(c,a.path);else throw"invalid path";},unhighlightAll:function(a){f(a).app.unhighlightAll()},highlightAndFade:function(a,b){var c=f(a),
d=c.app,e=d.getElement(c.path);return e?d.open(e.diagram,b).then(function(){d.highlightAndFade(e)}):(new h).reject("invalid path",!0)},enableModelToDoc:function(a){a=f(a);k(a.app)},disableModelToDoc:function(a){a=f(a);g[a.app.id]=!1},removeModelToDoc:function(a){a=f(a).app;l[a.id]&&(l[a.id].remove(),delete l[a.id]);g[a.id]=!1},isModelToDocEnabled:function(a){a=f(a);return n(a.app)},addDiagramLinkListeners:t("data-slwebview-diag-link",function(a){a=f(a);var b=a.app;a=b.getDiagram(a.path);g[b.id]=!1;
a?b.open(a,{nohash:!0}).then(function(){k(b)},function(){k(b)}):k(b)}),addElementLinkListeners:t("data-slwebview-elem-link",function(a){a=f(a);var b=a.app,c=b.getElement(a.path);g[b.id]=!1;c?b.open(c.diagram,{nohash:!0}).then(function(){b.moveToView(c);b.select(c);b.highlightAndFade(c);k(b)},function(){k(b)}):k(b)})}});
//# sourceMappingURL=interface.js.map