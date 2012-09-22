package org.hettoo.substitution;

public class Argument {
    private String name;
    private boolean optional;
    private boolean variable;

    public Argument(String name, boolean optional, boolean variable) {
        this.name = name;
        this.optional = optional;
        this.variable = variable;
    }

    public String getName() {
        return name;
    }

    public boolean isOptional() {
        return optional;
    }

    public boolean isVariable() {
        return variable;
    }

    public String toString() {
        String result = name;
        if (optional)
            result = "[" + name + "]";
        else
            result = "<" + name + ">";
        if (variable)
            result += "...";
        return result;
    }
}
