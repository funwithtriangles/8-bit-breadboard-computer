const fs = require('fs')
const commands = require('./commands')
const args = process.argv.slice(2)
const fileName = args[0]

const dec2bin = n => {
  n = parseFloat(n)
  if (n < 0 || n > 255 || n % 1 !== 0) {
      throw new Error(n + " does not fit in a byte")
  }
  return ("000000000" + n.toString(2)).substr(-8)
}

const shouldSkip = line => 
  line.length <= 0 ||
  line.trim()[0] === ';'

const processLine = line => {
  line = line.split(';')[0].trim()
  return {
    line,
    firstChar: line.substr(0, 1),
    lastChar: line.substr(-1),
  }
}

const processNum = num => 
  num.substr(0, 1) === 'b' ?
  num.substr(1) :
  dec2bin(num)

const labels = {}
const vars = {}

const lines = fs.readFileSync(fileName).toString().split("\n");

let currLine = 0;

// Find labels and variables
lines.forEach(l => {
  if (shouldSkip(l)) return
  const { line, firstChar, lastChar } = processLine(l)

  if (lastChar === ':') {
    const labelName = line.substr(0, line.length - 1)
    labels[labelName] = dec2bin(currLine)
  } else if (firstChar === '$') {
    const lineParts = line.split(' ')
    const varName = lineParts[0].substr(1)
    vars[varName] = processNum(lineParts[1])
  } else {
    currLine ++
  }
})

console.log(labels, vars)

let assembled = []

// Assemble
lines.forEach(l => {
  if (shouldSkip(l)) return
  const { line, firstChar, lastChar } = processLine(l)

  // If label or var, ignore
  if (lastChar === ':' || firstChar === '$') return

  let bin = labels[line] ?? vars[line] ?? commands[line]

  if (bin === undefined) {
    bin = processNum(line)
  }

  assembled.push(bin)
})

console.log('---------')
console.log(assembled.join('\n'))