var Shmdb = require('./build/Release/shmdb');
module.exports = function(size) {
	return new Shmdb(size);
};