import { arch, platform } from "os";

if (platform() === 'win32' || platform() === 'win64' || platform() == 'linux' || platform() == 'darwin') {
	console.log('pass');
} else {
	console.log('no pass, ' + platform());
}
if (arch() === 'x64' || arch() == 'x32' || arch() == 'unknown') {
	console.log('pass');
} else {
	console.log('no pass, ' + arch());
}
