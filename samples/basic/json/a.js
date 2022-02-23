// Stringify
console.log(JSON.stringify(0));
console.log(JSON.stringify(123));
console.log(JSON.stringify(-123));
console.log(JSON.stringify(undefined));
console.log(JSON.stringify(null));
console.log(JSON.stringify(""));
console.log(JSON.stringify("abc"));
console.log(JSON.stringify({}));
console.log(JSON.stringify([]));
console.log(JSON.stringify([1]));
console.log(JSON.stringify([1,2]));
console.log(JSON.stringify([1,2,3]));
console.log(JSON.stringify(['a']));
console.log(JSON.stringify(['a', 'b']));
console.log(JSON.stringify(['a', 'b', 'c']));
console.log(JSON.stringify({"name":"John","age":30,"city":"New York"}));

// Parse
console.log(JSON.parse("0"));
console.log(JSON.parse("123"));
console.log(JSON.parse("-123"));
console.log(JSON.parse("null"));
console.log(JSON.parse("\"\""));
console.log(JSON.parse("\"abc\""));
console.log(JSON.parse("{}"));
console.log(JSON.parse("[]"));
console.log(JSON.parse("[1]"));
console.log(JSON.parse("[1,2]"));
console.log(JSON.parse("[1,2,3]"));
console.log(JSON.parse('["a"]'));
console.log(JSON.parse('["a","b"]'));
console.log(JSON.parse('["a","b","c"]'));
console.log(JSON.parse('{"name":"John","age":30,"city":"New York"}'));
