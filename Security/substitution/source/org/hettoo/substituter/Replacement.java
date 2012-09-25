package org.hettoo.substituter;

import java.io.InputStream;
import java.io.Writer;
import java.io.IOException;

public class Replacement {
    private char target;
    private char result;

    public Replacement(char target, char result) {
        this.target = target;
        this.result = result;
    }

    public Replacement(InputStream in) throws IOException {
        this((char)in.read(), (char)in.read());
    }

    public void write(Writer out) throws IOException {
        out.write(target);
        out.write(result);
    }

    public char getTarget() {
        return target;
    }

    public boolean applies(char target) {
        return target == this.target;
    }

    public char getResult() {
        return result;
    }

    public void setResult(char result) {
        this.result = result;
    }

    public char apply(char target) {
        if (applies(target))
            return getResult();
        return target;
    }
}
