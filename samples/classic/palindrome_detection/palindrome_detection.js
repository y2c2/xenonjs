var isPalindrome = function (str) {
  return str === str.split('').reverse().join('');
};
 
console.log(isPalindrome("")); // true
console.log(isPalindrome("a")); // true
console.log(isPalindrome("ab")); // false
console.log(isPalindrome("aba")); // true
