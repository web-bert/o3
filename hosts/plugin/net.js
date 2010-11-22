(function (){
	if (!require.s.o3modules)
		require.s.o3modules = {};
	require.s.o3modules["net"] = o3NetFactory;
})();

function o3NetFactory(o3) {
	o3.require('socket');
	if(!o3.loaded)
		o3.loaded = {};
		
	o3.loaded.net = function(){
		return {createServer: createServer, createConnection: createConnection, Stream:Stream};
	}	
	
	_doCallbacks = function(self, eventType, arg) {
		var tmp = self.cb, cbGroup = tmp[eventType];
			
		if (cbGroup)
			for (var i=0; i<cbGroup.length; i++)
				cbGroup[i](arg);
	}	
	
	function on(self, eventType, cb) {
		var cbGroup;		
		cbGroup = self.cb[eventType];
		if (!cbGroup)
			cbGroup = self.cb[eventType] = [];
		
		cbGroup.push(cb);
	}

	function createServer(listener) {
		return new Server(listener);
	}
	
	function createConnection(port, host) {
		if (!host)
			host = '127.0.0.1';
		var s = new Stream();
		s.connect(port, host);
		return s;
	}
	
	function Server (listener) {
			
		var _self = o3.socketTCP();
		_self.maxConnections = 20;
		_self.connections = 0;
				
		_self.cb = {};
		
		_self.on = _self.addListener = function(eventType, cb) {
			on(_self, eventType, cb);
		}
		
		_self.onaccept = function(socket) {
			var s = new Stream(socket)
			listener(s);
			_doCallbacks(s, 'connect', s);
			_doCallbacks(_self, 'connection', s);
		}
		
		_self.onerror = function(socket) {
			_doCallbacks(_self,"error"); // error info here
		}
	
		_self.listen = function(port, host/*, callback*/) {
			//this.on('connection', callback);
			_self.bind(host, port);
			_self.accept();    
		}
		
		return _self;
	};
	
//Server.prototype.listen(path, [callback])
//	Server.prototype.close = function() {
	
//	}
	
	function Stream (socket) {

		var _self = socket ? socket : o3.socketTCP();

		_self.cb = {};
		//this.on('connection', listener);
		
		_self.on = _self.addListener = function(eventType, cb) {
			//'connection', 'close'; 
			//	'connect'
			//	'data'
				//The argument data will be a Buffer or String. Encoding of data is set by stream.setEncoding(). (See the section on Readable Stream for more information.)
			//	'end'
			//	'drain'
			//	'error'
			//	'close'
			on(_self, eventType, cb);
		}		

		
		_self.onconnect = function(socket) {
			_self.receive();
			_doCallbacks(_self,"connect", _self);
		}
		
		_self.onreceive = function(socket) {
			_doCallbacks(_self,"data", socket.receivedBuf);
			socket.clearBuf();
		}
		
		_self.onerror = function(socket) {
			_doCallbacks(_self,"error"); // error info here
		}
		
		_self.receive();
			
		_self.connect = function(port, host) {
			if (!host)
				host = '127.0.0.1';
			
			_self.onreceive = function(socket) {
				_doCallbacks(_self,"data", socket.receivedBuf);
				socket.clearBuf();
			}
			_self._connect(host, port);
		}
			
		_self.write = function(data, encoding) {
			if (!encoding)
				encoding = 'ascii';
		//Sends data on the stream. 
		//Returns true if the entire data was flushed successfully to the kernel buffer. 
		//Returns false if all or part of the data was queued in user memory. 
		//'drain' will be emitted when the buffer is again free.
			_self.send(data);
		}
		
		_self.end = function(data, encoding) {
		//Half-closes the stream. I.E., it sends a FIN packet. It is possible the server will still send some data. After calling this readyState will be 'readOnly'.
		//If data is specified, it is equivalent to calling stream.write(data, encoding) followed by stream.end().
		}
		
		_self.destroy = function() {
			_self.close();
		//Ensures that no more I/O activity happens on this stream. Only necessary in case of errors (parse error or so).
		}	
		
		return _self;
	};
	
}