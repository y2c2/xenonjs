var i = 0; var s = 0;
do {
  i += 1;
  s += i;
  if (i < 100) continue;
} while (false);
console.log(s); // 1

var i = 0; var s = 0;
while (true) {
  s += i;
  if (i == 100) break;
  i += 1;
}
console.log(s); // 5050

var i = 0; var s = 0;
while (true) {
  i += 1;
  s += i;
  if (i < 100) continue;
  break;
}
console.log(s); // 5050

var i = 0; var s = 0;
do {
  i += 1;
  s += i;
  if (i >= 100) break;
} while (true);
console.log(s); // 5050
