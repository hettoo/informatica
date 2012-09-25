package org.hettoo.cli;

import java.util.ArrayList;
import java.util.List;

public abstract class AbstractCommandHandler implements CommandHandler {
    private String name;
    private List<Argument> arguments;

    public AbstractCommandHandler() {
        arguments = new ArrayList<Argument>();
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getName() {
        return name;
    }

    protected void addArgument(Argument argument) {
        arguments.add(argument);
    }

    public void handle(List<String> arguments) {
        ArgumentProvider provider
            = new ArgumentProvider(this.arguments, arguments);
        if (provider.validate())
            handle(provider);
        else
            System.out.println("Usage: " + getName() + " " + getUsage());
    }

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

    protected abstract void handle(ArgumentProvider arguments);
}
