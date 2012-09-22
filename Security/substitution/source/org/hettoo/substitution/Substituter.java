package org.hettoo.substitution;

import java.util.ArrayList;
import java.util.List;
import java.util.ListIterator;

public class Substituter {
    private String original;
    private String current;
    protected final List<Replacement> replacements;

    public Substituter() {
        original = "";
        current = original;
        replacements = new ArrayList<Replacement>();
    }

    public void setOriginal(String original) {
        this.original = original;
        updateCurrent();
    }

    public void addOriginal(String extra) {
        setOriginal(original + extra);
    }

    public String getOriginal() {
        return original;
    }

    public boolean isReplaced(char c, boolean strict) {
        for (Replacement replacement : replacements) {
            if ((strict && replacement.getTarget() == c)
                    || (!strict && replacement.apply(c) != c))
                return true;
        }
        return false;
    }

    public void addReplacement(Replacement replacement) {
        for (Replacement e : replacements) {
            if (e.getTarget() == replacement.getTarget()) {
                e.setResult(replacement.getResult());
                updateCurrent();
                return;
            }
        }
        replacements.add(replacement);
        updateCurrent();
    }

    public void deleteReplacement(char target) {
        for (ListIterator<Replacement> i = replacements.listIterator();
                i.hasNext();) {
            if (i.next().getTarget() == target) {
                i.remove();
                updateCurrent();
            }
        }
    }

    protected void updateCurrent() {
        current = "";
        for (char c : original.toCharArray()) {
            for (Replacement replacement : replacements) {
                if (replacement.applies(c)) {
                    c = replacement.apply(c);
                    break;
                }
            }
            current += c;
        }
    }

    public String getCurrent() {
        return current;
    }

    public String getCurrentMasked(char mask) {
        String result = "";
        for (int i = 0; i < original.length(); i++) {
            if (isReplaced(original.charAt(i), true))
                result += current.charAt(i);
            else
                result += mask;
        }
        return result;
    }
}
