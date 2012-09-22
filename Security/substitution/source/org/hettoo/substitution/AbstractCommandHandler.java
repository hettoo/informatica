package org.hettoo.substitution;

import java.util.ArrayList;
import java.util.List;

public abstract class AbstractCommandHandler implements CommandHandler {
    private String name;
    private List<Argument> arguments;
    private int minimumArguments;
    private int maximumArguments;
    private boolean hasOptional;

    public AbstractCommandHandler() {
        arguments = new ArrayList<Argument>();
        minimumArguments = 0;
        maximumArguments = 0;
        hasOptional = false;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getName() {
        return name;
    }

    protected void addArgument(Argument argument) {
        if ((argument.isVariable() && (maximumArguments == -1 || hasOptional))
                || (argument.isOptional() && maximumArguments == -1)) {
            System.err.println("Multiple variable arguments or a combination of"
                    + " optional and variable arguments is not possible.");
            System.exit(1);
        }
        arguments.add(argument);
        if (!argument.isOptional())
            minimumArguments++;
        else
            hasOptional = true;
        if (argument.isVariable())
            maximumArguments = -1;
        if (maximumArguments != -1)
            maximumArguments++;
    }

    public void handle(List<String> arguments) {
        int size = arguments.size();
        if (size < minimumArguments || size > maximumArguments) {
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
