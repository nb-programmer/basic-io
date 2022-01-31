# Cloud BASIC interpreter

BASIC language interpreter as PaaS!

Run BASIC programs in your Web Browser by compiling it on the cloud, and sending the output back.

Note: This branch is a snapshot of the original submission I made for my University project.

## What is this?

A BASIC interpreter with a few commands. But it can be used through a web browser.

The BASIC code typed by the user is sent as a POST request to the BASIC server. Over there, the body is separated, and sent to various stages in the parser chain.

## How it works?

An HTTP POST request is handled by the server code that identifies request arguments if any (such as disabling LOG output, etc.) and extracts the body, which is the program code.

The code is sent to the following two stages to get parsed:

1. Lexer: Here, the various characters are interpreted as Tokens, and are assigned a token type based on what character it is, and what context it is in (eg: a string literal if it is between quotes, identifier if it fits the criteria, etc.)
2. Parser: This stage converts the list of tokens into an Abstract Syntax Tree (AST). This is a data structure that holds the sequence of instructions that the BASIC code comprises of.

After the AST is generated, it can be used for various things, such as syntax analysis, data analytics, etc. But we will use it to execute the instructions present in it.

The AST is sent to an Interpreter. This interpreter has its input and output set to the socket of the HTTP request. Hence, a `printf()` call will actually write back to the response to the client.
The interpreter traverses the AST in breadth-first order, calling functions on its way.

## Building

This code is made to build on a Unix-compatible machine (sorry Windows users). Just call `make` and it will output the server binary in `bin/server`. There are no other dependencies.

But before that, you need to have the two folders, `bin` and `obj` present in the project directory, if not present.

## Running

From the repository folder, run `bin/server`. Don't run it from the bin directory, as it requires the *static* folder to be present in the pwd (You can just move one or the other so that those two are in the same directory).

This will start an HTTP server on port `1111` (You can change this by changing `LISTEN_PORT` constant in `main.c`)

Start a browser and head over to [http://localhost:1111/](http://localhost:1111/). It will present to you a sort-of IDE where you can type in code and execute it.

You can also Load and Save your program on the cloud in one of 3 slots (shared by everyone using this server). This feature is just to demonstrate cloud saving.

## Language features

These are the features implemented into the language (some were removed due to time constraints at the time)

### Clauses

- `IF` – a condition to check, and execute part of a program if the condition is true
- `THEN` – must be used after specifying condition
- `ELSE` – program to execute if the condition in the IF statement is false. Optional.
- `END` – must be used to indicate the ending of IF program or ELSE program block
- `WHILE` – similar to IF, but jumps back to condition after the program block is
finished, till condition becomes FALSE

### Built-in constants

- `TRUE` / `FALSE` – aliases for 1 and 0 respectively
- `PI` – constant with value of 𝜋 – approximately 3.14159…
- `RANDOM_MAX` – value which is maximum return value of `IRANDOM()`. Depends
on the system that is running.

### Operations

The operations follow the BODMAS rule, in the same precedence as C programming
language.

- Arithmetic: `+, -, *, /, %`
  - Performs the respective operation on two operands
- Logical: `<, >, !, =`
  - Can be used to compare values or strings
- Assignment: `=`
  - '=' is assignment if the expression is not used for condition

Yes, `=` is used for both assignment and for comparison.

### Built-in functions

Functions are written that perform pre-defined instructions for the user. Currently, the language doesn't support user-defined functions.

- `PRINT()` – simple function that takes as input zero or more arguments, and displays
each in the output window.
- `MIN()` – takes as input one or more arguments of numbers and returns the smallest
number among them
- `MAX()` – takes as input one or more arguments of numbers and returns the largest
number among them
- `SLEEP()` – Waits in the current statement for given number of seconds. Argument
should be a number or float
- `INT()` – converts given argument to an integer representation. Any decimal point portion is trimmed-off.
- `FLOAT()` – converts given argument to a float representation
- `RANDOM()` – Returns a randomly generated float between 0 and 1
- `IRANDOM()` – Returns a randomly generated integer between 0 and RANDOM_MAX

**Note**: This version doesn't have any user-input capability, as it was written with HTTP requests in mind.

### Data-types

Three data-types are supported: Number (integers), Float (floating-point decimal) and Strings.
There is no need to indicate what data-type a variable is, as it is automatic.

### Variable declaration

Variables are declared by just assigning an identifier word with a value. The variable must be
named by beginning with a letter, and later on can include numbers and underscore. No
whitespace and special characters are allowed in the name.

## Example programs

Here are some example programs to try out the syntax of the language:

### Hello world program

```basic
print("Hello world!")
```

### Simple condition check

```basic
val = 5
if val < 10 then
 print("Success")
else
 print("Failure")
end
```

### Count up numbers

```basic
val = 0
while val < 10 then
 print("Counting at", val)
 val = val + 1
end
```
