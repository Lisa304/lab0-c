# Test of new version of sorting list_sort
option fail 0
option malloc 0
new
ih RAND 50000
time
timsort
time
free