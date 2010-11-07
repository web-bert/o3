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
		/(?:(normal|italic|oblique)\s*)?/ ,             // 1 style 
		/(?:(normal|small-caps|inherit)\s*)?/ ,         // 2 variant
		/(?:(normal|bold|bolder|lighter|[1-9]00)\s*)?/ ,// 3 weight
		/(?:([\d\.]+)?(px|pt|pc|in|cm|mm|%)\s*)?/ ,         // 4 5 size
		/(?:\/([\d\.]+)?(px|pt|pc|in|cm|mm|%)\s*)?/ ,       // 6 7 line height
		/(?:('[^']+'|"[^"]+"|\w[\w\s-]+\w))?/,          // 8+ Family and fallbacks
		fb,fb,fb,fb,fb,fb,fb 
	));

var fontscale = {
    pt : 1/0.75,
    in : 96,
    mm : 96/25.4,
    cm : 96/2.54
};

var fontlookup = {
	"arial": "arial.ttf"
}

module.exports = function(x,y,mode){
    
    var ctx = o3.canvas(x,y,mode);
    ctx.onSetFont = function(font){// need getter/setter overload in O3 too
       
       var m = fontcache[font] || (fontcache[font] = String(font).match(fontrx));
       if(!m) return;
       // ctx.fontStyle = m[0]; // and so on.
       if(m[4]) ctx.fontSize = parseFloat(m[4]) * (fontscale[m[5]] || 1);
	   if(m[8]) ctx.fontFamily =   fontlookup[m[8].toLowerCase()] || "arial.ttf";
	   
	   console.log("font parsing results: ");
	   console.log(m);
    };
    return ctx;
};