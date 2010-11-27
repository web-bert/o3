
ctx = o3.canvas(128,128,"argb");
var thetime = 0;
//RunOneTest('thin color lines');
//RunOneTest('clip() 2');

var done = false;

var gl = o3.createGLWindow("o3-OpenGL output",10,10,640,480);

var Tex = gl.texture(128, 128);
var innerrad = 0.3;
var outerrad = 0.5;
var toruswidth = outerrad-innerrad;
var toruscenter = (outerrad+innerrad)*0.5;
var index =0 ;

var SomeCube = gl.cube();

var K = o3.kinect();

var xvert = 32;
var yvert = 32;
var vertspersquare = 6;
var MntArray = gl.vertexarray(xvert*yvert*vertspersquare);
var FractalHeight = []

function FractalGenerate(xsize, ysize, octaves)
{
  // begin by initialising everywhere to 0
    for (var x = 0; x< xsize;x++)
  {
    FractalHeight[x]=[]
    for (var y = 0; y< ysize;y++)
    {
		FractalHeight[x][y]=0
	}
  }

  // add each octave to the fractal
  FractalMaxHeight = 0
  FractalAddOctave(xsize, ysize, octaves)

   for (var x = 0; x< xsize;x++)
  {
    for (var y = 0; y< ysize;y++)
    {
		FractalHeight[x][y]*=0.2
		FractalHeight[x][y]-=1;
	}
  }

}

function RandRange(min, max)
{
	return Math.random()*max;
};
// recursively adds octaves to the fractal
function FractalAddOctave(xsize, ysize, octave)
{
  // add this octave
  var step = Math.pow(2,(octave - 1));
  var scale = step
  FractalMaxHeight = FractalMaxHeight + scale
  for (var x = 0; x< xsize;x+=step)
  {
     for (var y = 0; y< ysize;y+=step)
	{
	  // Add random value at this point
//	  print(x,y,scale)
	  FractalHeight[x][y] = FractalHeight[x][y] + RandRange(0,scale)
	  //if we aren't on the last octave, interpolate intermediate values
	  if (step > 1)
	  {
	    if (x > 1)
		{
	      FractalHeight[x - step / 2][y] = (FractalHeight[x - step][y] + FractalHeight[x][y]) / 2
		}
        if (y > 1)
		{
		  FractalHeight[x][y - step / 2] = (FractalHeight[x][y - step] + FractalHeight[x][y]) / 2
		}
		if (x > 1 && y > 1)
		{
		  FractalHeight[x - step / 2][y - step / 2] = (FractalHeight[x - step / 2][y - step] + FractalHeight[x - step / 2][y]) / 2
		}
	  }
	}
  }

  // add the next octave
  if (octave > 1)
  {
    FractalAddOctave(xsize, ysize, octave - 1)
  };
  
}

FractalGenerate(xvert, yvert, 4);
function Normal(vv1, vv2, vv3)
{
	var vvA = [vv1[0], vv1[1], vv1[2]];
	vvA[0]-=vv2[0];
	vvA[1]-=vv2[1];
	vvA[2]-=vv2[2];
	
	var vvB = [vv1[0], vv1[1], vv1[2]];
	vvB[0]-=vv3[0];
	vvB[1]-=vv3[1];
	vvB[2]-=vv3[2];

	
	
	return [vvA[1]* vvB[2] - vvA[2] * vvB[1],
	vvA[2]* vvB[0] - vvA[0] * vvB[2],
	vvA[0]* vvB[1] - vvA[1] * vvB[0]];
}

index = 0;
var midx = -(xvert-1)/2;
var midy = -(yvert-1)/2;
var ws = 0.3;
for (var xx = 0;xx<xvert-1;xx++)
{
	for (var yy = 0;yy<yvert-1;yy++)
	{
		var vv1 = [(midx+xx)*ws,FractalHeight[xx][yy], (midy+yy)*ws];
		var vv2 = [(midx+xx+1)*ws,FractalHeight[xx+1][yy], (midy+yy)*ws];
		var vv3 = [(midx+xx+1)*ws,FractalHeight[xx+1][yy+1], (midy+yy+1)*ws];
		var vv4 = [(midx+xx)*ws,FractalHeight[xx][yy+1], (midy+yy+1)*ws];
		
		var n1 = Normal(vv1,vv2,vv3);
		var n2 = Normal(vv1,vv3,vv4);
		
		MntArray.setVertex(index , vv1[0],vv1[1],vv1[2],  n1[0],n1[1],n1[2],0,0); index++;
		MntArray.setVertex(index , vv2[0],vv2[1],vv2[2],  n1[0],n1[1],n1[2],0,0); index++;
		MntArray.setVertex(index , vv3[0],vv3[1],vv3[2],  n1[0],n1[1],n1[2],0,0); index++;

		MntArray.setVertex(index , vv1[0],vv1[1],vv1[2],  n2[0],n2[1],n2[2],0,0); index++;
		MntArray.setVertex(index , vv3[0],vv3[1],vv3[2],  n2[0],n2[1],n2[2],0,0); index++;
		MntArray.setVertex(index , vv4[0],vv4[1],vv4[2],  n2[0],n2[1],n2[2],0,0); index++;
	}                                                      
	
}

//var Teapot = LoadWavefrontObj("teapot.obj");
//var Plane = LoadWavefrontObj("cessna.obj");
function LoadFile(filename)
{
return o3.cwd.get(filename).data;
}

var mntvert = LoadFile("mntvert.txt");
var mntfrag =  LoadFile("mntfrag.txt");

var vert = "uniform float time;void main() { vec4 newpos = gl_Vertex;newpos.y += sin(time+newpos.x*2)*0.5; gl_Position = gl_ModelViewProjectionMatrix * newpos;}";
var frag = "void main(){ gl_FragColor = vec4(sin(gl_FragCoord.x/10),cos(gl_FragCoord.y/10),1.0,1.0);}"

var SimpleShader = gl.shaderprogram(vert, frag);
var MntShader = gl.shaderprogram(mntvert, mntfrag);

gl.onclose = function()
{
	done = true;
};

var T = 0;

while (!done)
{
	T+=0.01;
	gl.BeginFrame();
	gl.Viewport(0,0,gl.width, gl.height);
	gl.ClearColor(0.9,0.95,1.0,1);
	gl.Clear(gl.COLOR_BUFFER_BIT + gl.DEPTH_BUFFER_BIT);
	gl.MatrixMode(gl.PROJECTION);
	gl.LoadIdentity();
	gl.Perspective(35, gl.width/gl.height, 0.1, 1000);
	gl.MatrixMode(gl.MODELVIEW);
	gl.LoadIdentity();
	gl.LookAt(Math.cos(T)*10,2.7,Math.sin(T)*10,0,1.7,0,0,1,0);
	if (1)
	{
		gl.Enable(gl.LIGHTING);
		gl.Enable(gl.LIGHT0);
		gl.Enable(gl.LIGHT1);
		gl.Enable(gl.LIGHT2);

		gl.Light(gl.LIGHT0, gl.AMBIENT, 0.5, 0.5, 0.5, 1.0);
		gl.Light(gl.LIGHT0, gl.DIFFUSE, 1.0, 0.0, 0.0, 1.0);
		gl.Light(gl.LIGHT0, gl.SPECULAR, 1.0, 0.0, 0.0, 1.0);
		gl.Light(gl.LIGHT0, gl.POSITION, -1.5, 1.0, 4.0, 1.0);
		
		gl.Light(gl.LIGHT1, gl.AMBIENT, 0.0, 0.0, 0.0, 1.0);
		gl.Light(gl.LIGHT1, gl.DIFFUSE, 0.0, 1.0, 0.0, 1.0);
		gl.Light(gl.LIGHT1, gl.SPECULAR, 0.0, 1.0, 0.0, 1.0);
		gl.Light(gl.LIGHT1, gl.POSITION, -1.5, 1.0, -4.0, 1.0);

		gl.Light(gl.LIGHT2, gl.AMBIENT, 0.0, 0.0, 0.0, 1.0);
		gl.Light(gl.LIGHT2, gl.DIFFUSE, 0.0, 0.0, 1.0, 1.0);
		gl.Light(gl.LIGHT2, gl.SPECULAR, 0.0, 0.0, 1.0, 1.0);
		gl.Light(gl.LIGHT2, gl.POSITION, 1.5, 1.0, -4.0, 1.0);

		gl.Color3(0.5,0.5,0.5);
		gl.Enable(gl.CULL_FACE);
		gl.CullFace(gl.FRONT);
		gl.PushMatrix();
		gl.Render(MntArray);
		gl.PopMatrix();

		gl.Disable(gl.LIGHTING);
		gl.Disable(gl.LIGHT0);
		gl.Disable(gl.LIGHT1);
		gl.Disable(gl.LIGHT2);
	};
	if (0)
	{
		gl.PointSize(5);
		gl.Begin(gl.POINTS);
		gl.Color3(1,0,0);	gl.Vertex3(-1,-1,-1);
		gl.Color3(1,1,0);	gl.Vertex3( 1,-1,-1);
		gl.Color3(0,1,0);	gl.Vertex3( 1, 1,-1);
		gl.Color3(0,0,1);	gl.Vertex3(-1, 1,-1);
		gl.Color3(1,0.5,0);	gl.Vertex3(-1,-1, 1);
		gl.Color3(0.5,0,1);	gl.Vertex3( 1,-1, 1);
		gl.Color3(1,1,1);	gl.Vertex3( 1, 1, 1);
		gl.Color3(0.1,0.1,0.1);	gl.Vertex3(-1, 1, 1);
		gl.End();
	}
	
	if (0)
	{
		
		gl.Enable(gl.TEXTURE_2D);

		ctx.clear(0);
		var lingrad = ctx.createLinearGradient(0,0,0,128);
		lingrad.addColorStop(0, '#00ABEB');
		lingrad.addColorStop(1, '#fff');
		ctx.fillStyle = lingrad;
		ctx.fillRect(0,0,128,128);
  
		Tex.upload(ctx, 128,128);

		Tex.bind();
		
		gl.Enable(gl.LIGHTING);
		gl.Enable(gl.LIGHT0);
		gl.Enable(gl.LIGHT1);
		gl.Enable(gl.LIGHT2);

		gl.Light(gl.LIGHT0, gl.AMBIENT, 0.5, 0.5, 0.5, 1.0);
		gl.Light(gl.LIGHT0, gl.DIFFUSE, 1.0, 0.0, 0.0, 1.0);
		gl.Light(gl.LIGHT0, gl.SPECULAR, 1.0, 0.0, 0.0, 1.0);
		gl.Light(gl.LIGHT0, gl.POSITION, Math.cos(T*2.1)*4, 1.0, Math.sin(T*1.1)*4, 1.0);
		
		gl.Light(gl.LIGHT1, gl.AMBIENT, 0.0, 0.0, 0.0, 1.0);
		gl.Light(gl.LIGHT1, gl.DIFFUSE, 0.0, 1.0, 0.0, 1.0);
		gl.Light(gl.LIGHT1, gl.SPECULAR, 0.0, 1.0, 0.0, 1.0);
		gl.Light(gl.LIGHT1, gl.POSITION, Math.cos(T*1.2)*4, 1.0, Math.sin(T*1.2)*4, 1.0);

		gl.Light(gl.LIGHT2, gl.AMBIENT, 0.0, 0.0, 0.0, 1.0);
		gl.Light(gl.LIGHT2, gl.DIFFUSE, 0.0, 0.0, 1.0, 1.0);
		gl.Light(gl.LIGHT2, gl.SPECULAR, 0.0, 0.0, 1.0, 1.0);
		gl.Light(gl.LIGHT2, gl.POSITION, Math.cos(T*1.1)*4, 1.0, Math.sin(T*1.1)*4, 1.0);
		
		for (var x = -4;x<5;x++)
		{
			for (var y = -4;y<5;y++)
			{
				gl.PushMatrix();
				gl.Translate(x,5+y,0);
				gl.Color3(1,1,1);
				gl.Scale(0.5,0.5,0.5);				
				gl.Rotate(x*10,0,1,0);
				gl.Render(SomeArray);
				gl.PopMatrix();
				gl.PushMatrix();
				gl.Color3(0,0.5,0);
				gl.Translate(x,0,y);				
				gl.Render(SomeArray);
				gl.PopMatrix();
			};
		};
		
		gl.Disable(gl.LIGHTING);
		gl.Disable(gl.TEXTURE_2D);
		
	}
	
	if (0)
	{
		gl.PushMatrix();
		gl.Scale(0.2,0.2,0.2);
		gl.PushMatrix();
		gl.Color3(0.9,0.9,0.9);
		for (var i = 0;i<10;i++)
		{
			//SomeCube.Render(gl.TRIANGLES,0,18);
			SomeCube.Render();
			gl.Translate(1.5,0,0);
		};
		gl.PopMatrix();
		gl.Color3(0.9,0.9,0.2);
		gl.UseProgram(SimpleShader);
		gl.SetUniform(SimpleShader, "time", T*5);
		for (var i = 0;i<10;i++)
		{
			//SomeCube.Render(gl.TRIANGLES,0,18);
			gl.Render(SomeCube);
			gl.Translate(0,1.5,0);
		};
		gl.UseProgram(0);		
		gl.PopMatrix();
	}
	
	if (1)
	{
		
	};	
	gl.EndFrame();
	o3.wait(0);
	K.processEvents();
	thetime++;	
};

