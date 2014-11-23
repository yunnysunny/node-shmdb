var Shmdb = require('./build/Release/shmdb');
const LOG_LEVEL_MAP = {
	'NONE'		:0 ,
	'FAULT'		:1 ,
	'ERROR'     :2 ,
	'WARN'		:3 ,
	'INFO'      :4 ,
	'DEBUG'     :5 ,
	'TRACE'     :6
};
module.exports = function(param) {
	var option = {};
	if (process.env.LOG_LEVEL) {		
		var logLevel = LOG_LEVEL_MAP[process.env.LOG_LEVEL.toUpperCase()];
		if (logLevel >= 0) {
			console.log('log level',logLevel);
			option.logLevel = logLevel;
		}
	}
	return new Shmdb(param,option);
};