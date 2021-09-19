(*
 *  CS164 Fall 94
 *
 *  Programming Assignment 1
 *    Implementation of a simple stack machine.
 *
 *  Skeleton file
 *)

class List {
   cur: Object;
   next: List;
   size: Int <- 0;

   push(new_cur: Object): List {
      (let new_head: List <- (new List).init(cur, next) in {
         size <- size + 1;
         next <- new_head;
         cur <- new_cur;
         self;
      })
   };

   pop(): Object {
      if size = 0 then {
         abort();
      } else if (isvoid next) then {
         size <- size - 1;
         cur;
      } else
         (let old_cur: Object <- cur in {
            size <- size - 1;
            cur <- next.get_cur();
            next <- next.get_next();
            old_cur;
         })
      fi fi
   };

   init(new_cur: Object, new_next: List): List {
      if (isvoid new_next) then {
         size <- 1;
         cur <- new_cur;
         self;
      } else {
         size <- size + new_next.get_size();
         cur <- new_cur;
         next <- new_next;
         self;
      }
      fi
   };

   get_cur(): Object {
      cur
   };

   get_next(): List {
      next
   };

   get_size(): Int {
      size
   };
};

class Stack inherits IO {
   converter: A2I <- new A2I;
   data: List <- new List;

   push(value: Object): Stack {
      {
         data.push(value);
         self;
      }
   };
   pop(): Object {
      data.pop()
   };
   size(): Int {
      data.get_size()
   };
   print(): Object {
      (let i: Int <- 0, curr_list: List <- data in {
         while i < data.get_size() loop {
            case curr_list.get_cur() of
               str: String => out_string(str.concat("\n"));
               val: Int => out_string(converter.i2a(val).concat("\n"));
               obj: Object => {
                  abort();
                  1;
               };
            esac;
            i <- i + 1;
            curr_list <- curr_list.get_next();
         }
         pool;
      })
   };
};

class StackMachine {
   data: Stack <- (new Stack);
   converter: A2I <- (new A2I);
   push(str: String): Bool {
      if str = "+" then { data.push(str); true; }
      else if str = "s" then { data.push(str); true; }
      else if str = "d" then { data.print(); true; }
      else if str = "e" then { evaluate(); true; }
      else if str = "x" then false
      else { data.push(converter.a2i(str)); true; }
      fi fi fi fi fi
   };
   evaluate(): Object {
      if data.size() < 3 then 0
      else
      (let command: Object <- data.pop() in
         case command of
            str: String =>
               if str = "s" then
               (let a: Object <- data.pop(), b: Object <- data.pop() in {
                     data.push(a);
                     data.push(b);
                  }
               )
               else if str = "+" then                
                  (let a: Object <- data.pop(), b: Object <- data.pop() in
                  case a of 
                     val1: Int =>
                        case b of
                           val2: Int => data.push(val1 + val2);
                           obj: Object => { abort(); };
                        esac;
                     obj: Object => { abort(); };
                  esac
                  )
               else abort()
               fi fi;
            i: Int => push(converter.i2a(i));
            obj: Object => abort();
         esac
      )
      fi
   };
};

class Main inherits IO {
   converter: A2I <- new A2I;
   a: StackMachine <- (new StackMachine);
   main(): Object {
      (let input: String <- {out_string(">"); in_string();} in {
         while a.push(input) loop {
            out_string(">");
            input <- in_string();
         }
         pool;
      })
   };
};

