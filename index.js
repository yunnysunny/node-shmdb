var Shmdb = require('./build/Release/shmdb');
module.exports = function(param) {
	return new Shmdb(param);
};