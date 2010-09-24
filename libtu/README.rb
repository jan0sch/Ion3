Generic C code for red-black trees.
Copyright (C) 2000 James S. Plank

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

---------------------------------------------------------------------------
Jim Plank
plank@cs.utk.edu
http://www.cs.utk.edu/~plank

Department of Computer Science
University of Tennessee
107 Ayres Hall
Knoxville, TN 37996

615-974-4397
$Revision: 1.2 $
---------------------------------------------------------------------------

Rb.c and rb.h are files for doing general red-black trees.

The directory ansi contains ansi-standard c code for rb-trees (there
are some gross pointer casts there but don't worry about them.  They
work).  The directory non-ansi contains straight c code for rb-trees.
The ansi version can also be used by c++ (it has been tested).

Rb.h contains the typedef for red-black tree structures.  Basically, 
red-black trees are balanced trees whose external nodes are sorted
by a key, and connected in a linked list.  The following is how
you use rb.c and rb.h:

Include rb.h in your source.

Make_rb() returns the head of a red-black tree.  It serves two functions:
Its p.root pointer points to the root of the red-black tree.  Its 
c.list.flink and c.list.blink pointers point to the first and last 
external nodes of the tree.  When the tree is empty, all these pointers
point to itself.

The external nodes can be traversed in sorted order with their
c.list.flink and c.list.blink pointers.  The macros rb_first, rb_last,
rb_next, rb_prev, and rb_traverse can be used to traverse external node lists.

External nodes hold two pieces of information: the key and the value
(in k.key and v.val, respectively).   The key can be a character 
string, an integer, or a general pointer.  Val is typed as a character
pointer, but can be any pointer.  If the key is a character string, 
then one can insert it, and a value into a tree with rb_insert().  If
it is an integer, then rb_inserti() should be used.  If it is a general 
pointer, then rb_insertg() must be used, with a comparison function 
passed as the fourth argument.  This function takes two keys as arguments,
and returns a negative value, positive value, or 0, depending on whether
the first key is less than, greater than or equal to the second.  Thus,
one could use rb_insertg(t, s, v, strcmp) to insert the value v with 
a character string s into the tree t.

Rb_find_key(t, k) returns the external node in t whose value is equal
k or whose value is the smallest value greater than k.  (Here, k is
a string).  If there is no value greater than or equal to k, then 
t is returned.  Rb_find_ikey(t,k) works when k is an integer, and 
Rb_find_gkey(t,k,cmp) works for general pointers.

Rb_find_key_n(t, k, n) is the same as rb_find_key, except that it
returns whether or not k was found in n (n is an int *).  Rb_find_ikey_n
and Rb_find_gkey_n are the analogous routines for integer and general
keys.

Rb_insert_b(e, k, v) makes a new external node with key k and val v, and 
inserts it before the external node e.  If e is the head of a tree, 
then the new node is inserted at the end of the external node list.
If this insertion violates sorted order, no error is flagged.  It is 
assumed that the user knows what he/she is doing.  Rb_insert_a(e,k,v)
inserts the new node after e (if e = t, it inserts the new node at the
beginning of the list).

Rb_insert() is therefore really a combination of Rb_find_key() and
Rb_insert_b().

Rb_delete_node(e) deletes the external node e from a tree (and thus from 
the linked list of external nodes).  The node is free'd as well, so
don't retain pointers to it.

Red-black trees are spiffy because find_key, insert, and delete are all
done in log(n) time.  Thus, they can be freely used instead of hash-tables,
with the benifit of having the elements in sorted order at all times, and
with the guarantee of operations being in log(n) time.

Other routines:

Rb_print_tree() will grossly print out a red-black tree with string keys.
Rb_iprint_tree() will do the same with trees with integer keys.
Rb_nblack(e) will report the number of black nodes on the path from external
  node e to the root.  The path length is less than twice this value.
Rb_plength(e) reports the length of the path from e to the root.


You can find a general description of red-black trees in any basic algorithms
text.  E.g. ``Introduction to Algorithms'', by Cormen, Leiserson and Rivest
(McGraw Hill).  An excellent and complete description of red-black trees
can also be found in Chapter 1 of Heather Booth's PhD disseratation:
``Some Fast Algorithms on Graphs and Trees'', Princeton University, 1990.
