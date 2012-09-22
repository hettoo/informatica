package org.hettoo.substitution;

import java.util.ArrayList;
import java.util.List;

public class ArgumentProvider {
    private List<Argument> arguments;
    private List<String> actualArguments;
    private int variability;

    public ArgumentProvider(List<Argument> arguments,
            List<String> actualArguments) {
        this.arguments = arguments;
        this.actualArguments = actualArguments;
        variability = -1;
        for (int i = 0; i < arguments.size(); i++) {
            Argument argument = arguments.get(i);
            if (argument.isVariable())
                variability = i;
        }
    }

    private int find(String name) {
        for (int i = 0; i < arguments.size(); i++) {
            if (arguments.get(i).getName() == name)
                return i;
        }
        return -1;
    }

    public String getArgument(String name) {
        int index = find(name);
        if (index == -1 || index >= actualArguments.size())
            return null;
        if (variability == -1 || index < variability)
            return actualArguments.get(index);
        return actualArguments.get(arguments.size()
                - (actualArguments.size() - index));
    }

    public List<String> getVariableArgument() {
        List<String> arguments = new ArrayList<String>();
        for (int i = 0; i < actualArguments.size() - this.arguments.size(); i++)
            arguments.add(actualArguments.get(variability + i));
        return arguments;
    }
}
