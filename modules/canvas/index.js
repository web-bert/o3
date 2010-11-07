try{
	var o3 = require('./o3.js');
}catch(ex){
	var o3 = require('../o3.js');
}

function rxc(){
   return Array.prototype.map.call(arguments,function(b){return b.toString().slice(1,-1)}).join('');
}
var fontcache = {};
var fb = /(?:\s*,\s*('[^']+'|"[^"]+"|\w[\w\s-]+\w))?/
var fontrx = new RegExp(rxc(
        /^\s*/ ,
		/(?:(normal|italic|oblique)\s*)?/ ,             // 0 style 
		/(?:(normal|small-caps|inherit)\s*)?/ ,         // 1 variant
		/(?:(normal|bold|bolder|lighter|[1-9]00)\s*)?/ ,// 2 weight
		/(?:([\d\.]+)?(px|pt|pc|in|cm|mm|%)\s*)?/ ,         // 3 4 size
		/(?:\/([\d\.]+)?(px|pt|pc|in|cm|mm|%)\s*)?/ ,       // 5 6 line height
		/(?:('[^']+'|"[^"]+"|\w[\w\s-]+\w))?/,          // 7+ Family and fallbacks
		fb,fb,fb,fb,fb,fb,fb 
	));

var fontscale = {
    pt : 1/0.75,
    in : 96,
    mm : 96/25.4,
    cm : 96/2.54
};

module.exports = function(){
    
    var ctx = o3.canvas.apply(arguments);
    ctx.onParseFont = function(font){// need getter/setter overload in O3 too
       
       var m = fontcache[font] || (fontcache[font] = String(font).match(fontrx));
       if(!m) return;
       // ctx.fontStyle = m[0]; // and so on.
       if(m[3]) ctx.fontSize = parseFloat(m[3]) * (fontscale[m[4]] || 1);
    };
    return ctx;
    
};