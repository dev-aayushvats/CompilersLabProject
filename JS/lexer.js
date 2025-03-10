const Token = require('./token');

function lexer(input) {
  const tokens = [];
  let pos = 0;

  while (pos < input.length) {
    let char = input[pos];

    if (/\s/.test(char)) {
      pos++;
      continue;
    }

    if (/[a-zA-Z]/.test(char)) {
      let value = '';
      while (pos < input.length && /[a-zA-Z0-9]/.test(input[pos])) {
        value += input[pos++];
      }
      const keywords = ['int', 'for', 'while', 'do', 'if', 'else', 'switch', 'case', 'default', 'break'];
      tokens.push(new Token(keywords.includes(value) ? value.toUpperCase() : 'IDENTIFIER', value));
      continue;
    }

    if (/[0-9]/.test(char)) {
      let value = '';
      while (pos < input.length && /[0-9]/.test(input[pos])) {
        value += input[pos++];
      }
      tokens.push(new Token('NUMBER', value));
      continue;
    }

    const singleChars = {
      '+': 'PLUS', '-': 'MINUS', '*': 'MULT', '/': 'DIV', '%': 'MOD',
      '=': 'EQUALS', '<': 'LT', '>': 'GT', '(': 'LPAREN', ')': 'RPAREN',
      '{': 'LBRACE', '}': 'RBRACE', ';': 'SEMICOLON', ':': 'COLON', ',': 'COMMA'
    };
    if (singleChars[char]) {
      tokens.push(new Token(singleChars[char], char));
      pos++;
      continue;
    }

    if (char === '=' && input[pos + 1] === '=') {
      tokens.push(new Token('EQ', '=='));
      pos += 2;
      continue;
    }
    if (char === '!' && input[pos + 1] === '=') {
      tokens.push(new Token('NEQ', '!='));
      pos += 2;
      continue;
    }
    if (char === '<' && input[pos + 1] === '=') {
      tokens.push(new Token('LTE', '<='));
      pos += 2;
      continue;
    }
    if (char === '>' && input[pos + 1] === '=') {
      tokens.push(new Token('GTE', '>='));
      pos += 2;
      continue;
    }

    pos++;
  }

  console.log('Output from Lexer stage:', tokens.map(t => `(${t.type}, ${t.value})`));
  return tokens;
}

module.exports = lexer;