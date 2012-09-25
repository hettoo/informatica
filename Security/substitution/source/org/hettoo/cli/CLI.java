package org.hettoo.cli;

import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.ArrayList;
import java.util.List;
import java.util.Set;

public class CLI {
    private Map<String, CommandHandler> handlers;
    private CommandHandler unknownCommandHandler;
    private boolean stop;

    public CLI() {
        handlers = new LinkedHashMap<String, CommandHandler>();
    }

    public void stop() {
        stop = true;
    }

    public void run() {
        stop = false;
        while (!stop)
            requestCommand();
    }

    public Set<String> getCommands() {
        return handlers.keySet();
    }

    public CommandHandler getCommandHandler(String command) {
        return handlers.get(command);
    }

    public CommandHandler getUnknownCommandHandler() {
        return unknownCommandHandler;
    }

    protected String getInputIndicator() {
        return "> ";
    }

    protected void requestCommand() {
        System.out.print(getInputIndicator());
        List<String> arguments = new ArrayList<String>();
        boolean done = false;
        boolean quoted = false;
        boolean escaped = false;
        boolean next = true;
        for (;;) {
            int c = 0;
            try {
                c = System.in.read();
            } catch (Exception e) {
                e.printStackTrace();
                System.exit(1);
            }
            switch (c) {
                case '\\':
                    if (!escaped) {
                        escaped = true;
                        break;
                    }
                case '\'':
                    if (!escaped) {
                        quoted = !quoted;
                        break;
                    }
                case '\n':
                    if (!quoted && !escaped) {
                        execute(arguments);
                        return;
                    }
                case ' ':
                case '\t':
                    if (!quoted && !escaped) {
                        next = true;
                        break;
                    }
                default:
                    if (next)
                        arguments.add("");
                    arguments.set(arguments.size() - 1,
                            arguments.get(arguments.size() - 1) + (char)c);
                    escaped = false;
                    next = false;
                    break;
            }
        }
    }

    public void execute(List<String> arguments) {
        if (arguments.size() == 0)
            return;

        CommandHandler handler = handlers.get(arguments.get(0));
        if (handler == null)
            handler = unknownCommandHandler;
        else
            arguments.remove(0);
        if (handler != null)
            handler.handle(arguments);
    }

    public void setCommandHandler(String command, CommandHandler handler) {
        handler.setName(command);
        handlers.put(command, handler);
    }

    public void setUnknownCommandHandler(CommandHandler handler) {
        unknownCommandHandler = handler;
    }
}
