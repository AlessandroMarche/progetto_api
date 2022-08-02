# API Project: Wordchecker
#### ~ by Alessandro Marchetti


## What I use

### Hashmap 
For containing all the dictionary. 
I will use the the hashed string as the key and a unsigned char (0 or 1) as value.

If value is 0 the string should be excluded, otherwise the string should be considered as valid

The initial **size** of the hashtable will be 10000 items, but the table can be expanded (each time by 5000 items) is it is full


* 2^32 elements