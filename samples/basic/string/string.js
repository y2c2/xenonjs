// length

var s = "";
console.log(s);
console.log(s.length);
s = "abc";
console.log(s);
console.log(s.length);
s = "abc" + "123";
console.log(s);
console.log(s.length);

// charAt

var s = "abc";
console.log(s.charAt()); // "a"
console.log(s.charAt(0)); // "a"
console.log(s.charAt(1)); // "b"
console.log(s.charAt(2)); // "c"
console.log(s.charAt(3)); // ""

// split

var s = "abc";
console.log(s.split()); // ["abc"]
console.log(s.split('')); // ['a','b','c']
console.log(s.split('b')); // ['a','c']
console.log(s.split('a')); // ['bc']
console.log(s.split('c')); // ['ab']
console.log(s.split(' ')); // ['abc']

// indexOf

console.log('Blue Whale'.indexOf('Blue'));     // 0
console.log('Blue Whale'.indexOf('Blute'));    // returns -1
console.log('Blue Whale'.indexOf('Whale', 0)); // returns  5
console.log('Blue Whale'.indexOf('Whale', 5)); // returns  5
console.log('Blue Whale'.indexOf('Whale', 7)); // returns -1
console.log('Blue Whale'.indexOf(''));         // returns  0
console.log('Blue Whale'.indexOf('', 9));      // returns  9
console.log('Blue Whale'.indexOf('', 10));     // returns 10
console.log('Blue Whale'.indexOf('', 11));     // returns 10

console.log('Blue Whale'.indexOf('blue'));     // returns -1

// substr
var aString = 'Mozilla';

console.log(aString.substr(0, 1));   // 'M'
console.log(aString.substr(1, 0));   // ''
console.log(aString.substr(-1, 1));  // 'a'
console.log(aString.substr(1, -1));  // ''
console.log(aString.substr(-3));     // 'lla'
console.log(aString.substr(1));      // 'ozilla'
console.log(aString.substr(-20, 2)); // 'Mo'
console.log(aString.substr(20, 2));  // ''

// []
console.log("abc"[-1]); // undefined
console.log("abc"[0]); // a
console.log("abc"[1]); // b
console.log("abc"[2]); // c
console.log("abc"[3]); // undefined

// padStart
console.log('abc'.padStart(10));         // "       abc"
console.log('abc'.padStart(10, "foo"));  // "foofoofabc"
console.log('abc'.padStart(6,"123465")); // "123abc"
console.log('abc'.padStart(8, "0"));     // "00000abc"
console.log('abc'.padStart(1));          // "abc"
