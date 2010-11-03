/*
 * Copyright (C) 2010 Ajax.org
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this library; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

var canvasFactory = require('../index.js')
function drawtocontext(ctx)
{
	ctx.fillStyle = "rgb(200,200,200)";
	ctx.fillRect(0,0,300,300);
	
	ctx.fillStyle = "blue";
	ctx.strokeStyle = "black";
	ctx.globalAlpha = 0.5;
	ctx.strokeStyle= "rgb(0,0,0)";
	var n = Date.now()/1000;
	var xs = 20, ys = 20, cx = xs/2, cy=ys/2, xd = 300/xs, yd = 300/ys;
	for(var x = -cx, xt = 0; x < cx; x++, xt+=xd){
		for(var y = -cy, yt = 0; y < cy; y++, yt+=yd){
		    var sz = Math.sin(Math.sqrt(x*x+y*y)-n)*4+5;
			ctx.fillRect (xt-sz, yt-sz, 2*sz, 2*sz);
    		ctx.strokeRect (xt-sz, yt-sz, 2*sz, 2*sz);
		}
	}
}
/*
function draw() 
{
    var canvas = document.getElementById("canvas");
    var ctx = canvas.getContext("2d");
    drawtocontext(ctx);
}
*/
  
var http = require('http');
http.createServer(function (req, res) {
    
  var ctx = canvasFactory(300,300, "argb");
  drawtocontext(ctx);
  var buf = ctx.pngBuffer();
  var buf2 = ctx.jpgBuffer();
  res.writeHead(200, {'Content-Type': 'text/html'});
  res.end('<meta http-equiv="refresh" content="0.3;"><img alt="Embedded Image" src="data:image/png;base64,'+buf.toBase64()+'"><'
   +'<img alt="Embedded Image" src="data:image/png;base64,'+buf2.toBase64()+'">'
   );
}).listen(4000, "127.0.0.1");
console.log('Server running at http://127.0.0.1:4000/');



