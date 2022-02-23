var i = 0; var s = 0;
for (;;) {
  i += 1;
  s += i;
  if (i == 100) break;
}
console.log(s); // 5050

var i = 0; var s = 0;
for (;i < 100;) {
  i += 1;
  s += i;
}
console.log(s); // 5050

var i = 0; var s = 0;
for (;i <= 100; i++) {
  s += i;
}
console.log(s); // 5050

var i; var s = 0;
for (i = 0; i <= 100; i++) {
  s += i;
}
console.log(s); // 5050
