class StackCommand {
    parameter: String;

    init(parameter_: String): SELF_TYPE {
        {
            parameter <- parameter_;
            self;
        }
    };

    get_param(): String { parameter };

    -- Interface for all commands.
    run(stack: Stack): Object { abort() };
    display(): String { { abort(); "unknown";} };
    eval(stack: Stack): Object { abort() };
};

class NoopCommand inherits StackCommand {
    run(stack: Stack): Object { 1 };
    display(): String { { abort(); "unknown";} };
    eval(stack: Stack): Object { abort() };
};

class PlusCommand inherits StackCommand {
    run(stack: Stack): Object {
        stack.push(self)
    };

    display(): String { get_param() };

    eval(stack: Stack): Object {
        let first_int: Int <- read_int(stack),
            second_int: Int <- read_int(stack),
            sum: Int <- (first_int + second_int),
            result_cmd: IntegerCommand <- (new IntegerCommand).init((new A2I).i2a(sum))
        in
            stack.push(result_cmd)
    };

    read_int(stack: Stack): Int {
        case stack.pop() of
            intCommand: IntegerCommand => intCommand.to_int();
            o: Object => {abort(); 0;};
        esac
    };
};

class IntegerCommand inherits StackCommand {
    data: Int;

    run(stack: Stack): Object {
        stack.push(self)
    };

    display(): String { get_param() };

    eval(stack: Stack): Object {
        stack.push(self)
    };

    to_int(): Int { (new A2I).a2i(get_param()) };
};

class SwapCommand inherits StackCommand {
    run(stack: Stack): Object {
        stack.push(self)
    };

    display(): String { get_param() };

    eval(stack: Stack): Object {
        let first_cmd: Object <- stack.pop(),
            second_cmd: Object <- stack.pop()
        in {
            stack.push(first_cmd);
            stack.push(second_cmd);
        }
    };
};

class EvalCommand inherits StackCommand {
    run(stack: Stack): Object {
        if (not (stack.empty())) then
            let cmd: StackCommand <-
                case stack.pop() of
                    command: StackCommand => command;
                    o: Object => {abort(); (new StackCommand); };
                esac
            in
                cmd.eval(stack)
        else
            1
        fi
    };
};

class DisplayCommand inherits StackCommand {
    run(stack: Stack): Object {
        let head: StackNode <- stack.to_list() in
            while (not (isvoid head)) loop {
                let current_item: StackCommand <-
                    case head.get_item() of
                        stack_cmd: StackCommand => stack_cmd;
                        o: Object => {abort(); (new StackCommand); };
                    esac
                in
                    (new IO).out_string(current_item.display().concat("\n"));
                head <- head.get_next();
            }
            pool
    };
};
