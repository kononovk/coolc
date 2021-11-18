class A {};
class B {};

class Main inherits IO {
    main(): Object {
      1
    };

    foo(obj: Object): Object {
      case obj of
        a: A => out_string("This is A.");
        b: B => out_string("This is B.");
        c: Object => out_string("Unknown case.");
      esac
    };
};
