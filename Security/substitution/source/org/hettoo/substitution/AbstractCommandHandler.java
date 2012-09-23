package org.hettoo.substitution;

import java.util.ArrayList;
import java.util.List;

public abstract class AbstractCommandHandler implements CommandHandler {
    private String name;
    private List<Argument> arguments;
    private int minimumArguments;
    private int maximumArguments;
    private boolean optional;

    public AbstractCommandHandler() {
        arguments = new ArrayList<Argument>();
        minimumArguments = 0;
        maximumArguments = 0;
        optional = false;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getName() {
        return name;
    }

    protected void addArgument(Argument argument) {
        if (optional && !argument.isOptional()) {
            System.err.println("All optional arguments must go at the end.");
            System.exit(1);
        } else if (maximumArguments == -1) {
            System.err.println("The variable argument must be the last one.");
            System.exit(1);
        }
        arguments.add(argument);
        if (argument.isOptional())
            optional = true;
        else
            minimumArguments++;
        if (argument.isVariable())
            maximumArguments = -1;
        if (maximumArguments != -1)
            maximumArguments++;
    }

    public void handle(List<String> arguments) {
        int size = arguments.size();
        if (size < minimumArguments
                || (maximumArguments != -1 && size > maximumArguments)) {
            System.out.println("Usage: " + getName() + " " + getUsage());
            return;
        }
        handle(new ArgumentProvider(this.arguments, arguments));
    }

    protected abstract void handle(ArgumentProvider arguments);

    public String getUsage() {
        String usage = "";
        boolean first = true;
        for (Argument argument : arguments) {
            if (!first)
                usage += " ";
            else
                first = false;
            usage += argument;
        }
        return usage;
    }
}
