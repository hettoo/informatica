package org.hettoo.substitution;

import java.util.List;

public interface CommandHandler {
    public void setName(String name);
    public void handle(List<String> arguments);
    public String getDescription();
    public String getName();
    public String getUsage();
}
