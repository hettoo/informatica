package org.hettoo.substitution;

import java.util.TreeMap;
import java.util.Comparator;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.ArrayList;
import java.util.List;
import java.util.Scanner;
import java.util.Arrays;

import java.util.regex.Pattern;
import java.util.regex.MatchResult;

import java.io.BufferedInputStream;
import java.io.FileInputStream;
import java.io.FileWriter;

public class InteractiveSubstituter extends Substituter {
    private Map<String, CommandHandler> handlers;
    private CommandHandler unknownCommandHandler;
    private boolean stop;

    public InteractiveSubstituter() {
        handlers = new LinkedHashMap<String, CommandHandler>();
        setCommandHandler("set", new SetCommandHandler());
        setCommandHandler("show", new ShowCommandHandler());
        setCommandHandler("mask", new MaskCommandHandler());
        setCommandHandler("both", new BothCommandHandler());
        setCommandHandler("count", new CountCommandHandler());
        setCommandHandler("add", new AddCommandHandler());
        setCommandHandler("delete", new DeleteCommandHandler());
        setCommandHandler("clear", new ClearCommandHandler());
        setCommandHandler("list", new ListCommandHandler());
        setCommandHandler("read", new ReadCommandHandler());
        setCommandHandler("write", new WriteCommandHandler());
        setCommandHandler("stop", new StopCommandHandler());
        setCommandHandler("help", new HelpCommandHandler());
        setUnknownCommandHandler(new UnknownCommandHandler());
    }

    protected class StopCommandHandler implements CommandHandler {
        public void handle(List<String> arguments) {
            stop = true;
        }

        public String getDescription() {
            return "stops the interactive substituter";
        }
    }

    protected class HelpCommandHandler implements CommandHandler {
        public void handle(List<String> arguments) {
            System.out.println("Quote arguments using single quotes");
            System.out.println("Available commands:");
            for (String command : handlers.keySet())
                System.out.println(command + " - "
                        + handlers.get(command).getDescription());
        }

        public String getDescription() {
            return "get started";
        }
    }

    protected class UnknownCommandHandler implements CommandHandler {
        public void handle(List<String> arguments) {
            System.out.println("Unknown command " + arguments.get(0)
                    + ", try help");
        }

        public String getDescription() {
            return "rejects a command";
        }
    }

    protected class SetCommandHandler implements CommandHandler {
        public void handle(List<String> arguments) {
            setOriginal(arguments.get(0));
        }

        public String getDescription() {
            return "sets the original data";
        }
    }

    protected class ShowCommandHandler implements CommandHandler {
        public void handle(List<String> arguments) {
            System.out.println(getCurrent());
        }

        public String getDescription() {
            return "shows the current data";
        }
    }

    protected class MaskCommandHandler implements CommandHandler {
        public void handle(List<String> arguments) {
            System.out.println(getCurrentMasked('?'));
        }

        public String getDescription() {
            return "shows the current data with unreplaced data masked";
        }
    }

    protected class BothCommandHandler implements CommandHandler {
        public void handle(List<String> arguments) {
            execute(new ArrayList<String>(Arrays.asList("show")));
            execute(new ArrayList<String>(Arrays.asList("mask")));
        }

        public String getDescription() {
            return "shows the current data both unmasked and masked";
        }
    }

    protected class ListCommandHandler implements CommandHandler {
        public void handle(List<String> arguments) {
            for (Replacement replacement : replacements)
                System.out.println(replacement.getTarget() + " -> "
                        + replacement.getResult());
        }

        public String getDescription() {
            return "shows all replacements";
        }
    }

    protected class AddCommandHandler implements CommandHandler {
        public void handle(List<String> arguments) {
            addReplacement(new Replacement(arguments.get(0).charAt(0),
                        arguments.get(1).charAt(0)));
        }

        public String getDescription() {
            return "adds a replacement";
        }
    }

    protected class DeleteCommandHandler implements CommandHandler {
        public void handle(List<String> arguments) {
            deleteReplacement(arguments.get(0).charAt(0));
        }

        public String getDescription() {
            return "deletes a replacement";
        }
    }

    protected class ClearCommandHandler implements CommandHandler {
        public void handle(List<String> arguments) {
            replacements.clear();
        }

        public String getDescription() {
            return "clears all replacements";
        }
    }

    protected class ReadCommandHandler implements CommandHandler {
        public void handle(List<String> arguments) {
            try {
                BufferedInputStream in = new BufferedInputStream(
                        new FileInputStream(arguments.get(0)));
                boolean done = false;
                do {
                    in.mark(1);
                    int c = in.read();
                    in.reset();
                    if (c == -1)
                        done = true;
                    else
                        addReplacement(new Replacement(in));
                } while (!done);
                in.close();
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        public String getDescription() {
            return "reads replacements from a file";
        }
    }

    protected class WriteCommandHandler implements CommandHandler {
        public void handle(List<String> arguments) {
            try {
                FileWriter out = new FileWriter(arguments.get(0));
                for (Replacement replacement : replacements)
                    replacement.write(out);
                out.close();
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        public String getDescription() {
            return "writes replacements to a file";
        }
    }

    protected class CountCommandHandler implements CommandHandler {
        public void handle(List<String> arguments) {
            String original = getOriginal();
            HashMap<Character, Integer> map = new HashMap<Character, Integer>();
            TreeMap<Character, Integer> orderedMap
                = new TreeMap<Character, Integer>(new ValueComparator(map));
            for (int i = 0; i < original.length(); i++) {
                Character c = original.charAt(i);
                if (map.get(c) == null)
                    map.put(c, 1);
                else
                    map.put(c, map.get(c) + 1);
            }
            orderedMap.putAll(map);
            for (Character c : orderedMap.keySet()) {
                System.out.println((isReplaced(c, false) ? "x" : "o")
                        + " " + c + ": " + map.get(c));
            }
        }

        public String getDescription() {
            return "shows original character counts";
        }

        protected class ValueComparator implements Comparator<Character> {
            Map<Character, Integer> base;

            public ValueComparator(Map<Character, Integer> base) {
                this.base = base;
            }

            public int compare(Character a, Character b) {
                if (base.get(a) >= base.get(b))
                    return 1;
                return -1;
            }
        }
    }

    public void start() {
        stop = false;
        while (!stop)
            requestCommand();
    }

    private void requestCommand() {
        System.out.print("> ");
        List<String> arguments = new ArrayList<String>();
        arguments.add("");
        boolean done = false;
        boolean quoted = false;
        boolean escaped = false;
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
                    escaped = !escaped;
                    break;
                case '\'':
                    if (!escaped) {
                        quoted = !quoted;
                        break;
                    }
                case '\n':
                    if (!quoted) {
                        execute(arguments);
                        return;
                    }
                case ' ':
                case '\t':
                    if (!quoted) {
                        if (!arguments.get(arguments.size() - 1).equals(""))
                            arguments.add("");
                        break;
                    }
                default:
                    arguments.set(arguments.size() - 1,
                            arguments.get(arguments.size() - 1) + (char)c);
                    break;
            }
        }
    }

    private void execute(List<String> arguments) {
        CommandHandler handler = handlers.get(arguments.get(0));
        if (handler == null)
            handler = unknownCommandHandler;
        else
            arguments.remove(0);
        if (handler != null)
            handler.handle(arguments);
    }

    public void setCommandHandler(String command, CommandHandler handler) {
        handlers.put(command, handler);
    }

    public void setUnknownCommandHandler(CommandHandler handler) {
        unknownCommandHandler = handler;
    }
}
