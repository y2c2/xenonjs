var FirstFactorial = function (num) {
    if (num <= 1) return num;
    else return FirstFactorial(num - 1) * num;
};
console.log(FirstFactorial(4)); // 24
console.log(FirstFactorial(8)); // 40320
