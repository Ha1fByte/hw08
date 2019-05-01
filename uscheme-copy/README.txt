Names: Natalie Cardinali and Sean Francis
Project Choice : Copy Garbage Collector for microScheme
Time spent on assignment : 6hrs
Additional Collaborators:

... description of your solution to the problem ...
Using our knowlege of how the copy gargbage collector works we created a copy function and added to the collect function.
The copy function is called from the collect function, and it moves values from the fromspace to the tospace and then swaps the pointers to each space.
The collect function is run when allaloc finds that the heap pointer is equal to the heaplimit. It calls collect which first checks if the heap is initalized.  If not, it initializes the space then continues on, copying the roots and then having the scanloc pointer itterate through all of the copied values to see if they have any pointers pointing outside the tospace. If so, those values are forwarded over. This continues unitl the scanloc pointer meets up with the heap pointer. 

After all the values are copied and the spaces are swapped the collect function checks to make sure that the heap pointer isn't still equal to the heaplimit. If that's the case, that means that all the object on the heap are live and that the heap is too small to hold them all. The collect function then increases the size of the tospace, then copies all the values in the from spcae over like it did before. After the values are in the new, larger tospace, the program increases the size of smaller fromspace so it's the same size as the to space. 
Then it swaps the pointer like in normal execution and returns to allaloc. 


... description of how you tested your submission ...
... ( why are you convinced that it works correctly ) ...
... description of your submission 's functionality ...
... ( what is working , what is not working ) ...
