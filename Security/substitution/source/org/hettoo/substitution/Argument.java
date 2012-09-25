package org.hettoo.substitution;

import java.util.ArrayList;
import java.util.List;

public class Argument {
    private String name;
    private boolean optional;
    private boolean variable;
    private List<String> values;

    public Argument(String name, boolean optional, boolean variable) {
        this.name = name;
        this.optional = optional;
        this.variable = variable;
        values = new ArrayList<String>();
    }

    public void addValue(String value) {
        values.add(value);
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
        if (values.size() > 0) {
            boolean first = true;
            for (String value : values) {
                result += " " + (first ? "=" : "|") + " " + value;
                first = false;
            }
        }
        if (optional)
            result = "[" + result + "]";
        else
            result = "<" + result + ">";
        if (variable)
            result += "...";
        return result;
    }
}
