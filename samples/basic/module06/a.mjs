import os from "os";

if (os.platform() === 'win32' || os.platform() === 'win64' || os.platform() == 'linux' || os.platform() == 'darwin') {
	console.log('pass');
} else {
	console.log('no pass, ' + os.platform());
}
if (os.arch() === 'x64' || os.arch() == 'x32' || os.arch() == 'unknown') {
	console.log('pass');
} else {
	console.log('no pass, ' + os.arch());
}
