**# Project 3 Design Log**
**\*\*Author:\*\*** \<your name(s)> · **\*\*P2 code used:\*\*** \<my own>
**\*\*Tools used:\*\*** \<Claude - used for conceptually understanding what was needed to be done in tool_registry.cpp,&#x20;
as well as ideas for what should be done to make most of the test_p3 cases (and general review on testcases in c++).>


## 1. Hash table

The registry uses separate chaining, where each bucket has a vector of Entries. Each Entry stores the tool's name and a std::unique_ptr<Tool>. I chose this instead of linear probing because it is easier to handle when the table gets full, and it doesn't need special empty slots. It also won't get stuck searching forever if the table fills up. With a good hash, the number of probes should stay pretty low (6 compared to around 30 for linear probing at 0.75 load factor). I also store the name in the Entry so I don't have to call name() every time I compare tools, since that creates a string.
For hashing, I used 64-bit FNV-1a. It goes through each character, XORs it with the current hash, and multiplies by the FNV prime. The order of characters matters, so "ab" and "ba" get different hashes. The hash function returns the full hash value.

The bucket count starts at 8 and stays a power of two, which lets me use hash & (buckets - 1) to get the bucket index instead of using modulo.

## 2. Pointer stability

I used std::unique_ptr<Tool> to store the tools instead of putting the Tool objects directly into the buckets. This means each Tool stays in the same place in memory even when the hash table gets resized.
When rehashing, the entries are moved into new buckets. The unique_ptr gets moved, but the actual Tool object is still at the same address. So a Tool* returned from find() should still work after a resize.
The reg_10000_tools test checks this. It saves the pointer from the first insert, adds 9,999 more tools, and checks the pointer again after all the resizing. This makes sure the pointer didn't change.

## 3. Calculator trace

For this example, the expression is 2 * (3 + 4) - 5.

- 2: Put 2 onto the values stack, so values = [2].
- *: Put * onto the ops stack because it's empty.
- `(`: Push the opening parenthesis onto ops, so ops = [*, (].
- 3: Push 3 onto values, giving values = [2, 3]
- +: The top of ops is (, so I don't calculate anything yet. Push + onto ops
- 4: Push 4 onto values, so values = [2, 3, 4]
- ): Calculate what's inside the parentheses. Pop +, then pop 4 and 3. Calculate 3 + 4 = 7 and push 7 onto values. Remove ( from ops. Now values = [2, 7] and ops = [*].
- -: Since `*` has a higher precedence than `-`, calculate 2 * 7 = 14 first. Then push - onto ops.
- 5: Push 5 onto values, giving values = [14, 5].
- End: Calculate 14 - 5 = 9. The final answer is 9.

The >= rule makes the calculator do multiplication before subtraction in this example. It also makes subtraction happen from left to right, so 10 - 4 - 3 gives 3 instead of 9.

## 4. Errors as values

A tool failing doesn't always mean the program has a bug (a file might not exist, path might be outside the workspace, etc., which are all things that can happen during normal use)
Returning ToolResult:failure(...), or returning false with an error message from resolve and calc_evaluate, lets the program pass the error back normally. This way, the error can be shown in the conversation instead of stopping the whole program.
Exceptions make more sense for actual programming mistakes, like calling as_string() on a JSON number. That should be treated as a bug instead of a normal error the model is expected to fix. The thing still catches exceptions from tools in case something goes wrong.

## 5. What I would design differently

One thing I would change is storing the full hash value in each Entry along with the name. Right now, rehash has to calculate the hash again for every name when the buckets change. If I saved the hash, it could just use the hash value and the new bucket count to find the new index.

It would also let find() compare hash values before comparing the actual names, which could avoid some string comparisons when multiple tools end up in the same bucket.

I would also start the project earlier so I had more time to work through everything and test it.