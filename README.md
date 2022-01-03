# M.P.'s Editor

Morgan Peterson's Editor

## Derivation

MPE is based on the public domain code of Anthony Howe's editor (commonly known as Anthony's Editor or AE) and Hugh Barney's Atto/Femto.  Rather than representing a file as a linked list of lines, the AE Editor uses the concept of a Buffer-Gap.  A Buffer-Gap editor stores the file in a single piece of contiguous memory with some extra unused space known as the buffer gap.  On character insertion and deletion the gap is first moved to the current point.  A character deletion then extends the gap by moving the gap pointer back by 1 OR the gap is reduced by 1 when a character is inserted.  The Buffer-Gap technique is elegant and significantly reduces the amount of code required to load a file, modify it and redraw the display.

There is regex code in the editor that is derived from Marc André Tanner's Vis editor.

## Key Bindings
    C-A   begining-of-line
    C-B   backward-character
    C-D   delete-char
    C-E   end-of-line
    C-F   forward Character
    C-G   Abort (at prompts)
    C-H   backspace
    C-K   kill-to-eol
    C-N   next line
    C-P   previous line
    C-R   search-backwards
    C-S	  search-forwards
    C-U   undo
    C-V   Page Down
    C-W   Kill Region (Cut)
    C-X   CTRL-X command prefix
    C-Y   Yank (Paste)

    M-<   Start of file
    M->   End of file
    M-v   Page Up
    M-f   Forward Word
    M-b   Backwards Word
    M-g   goto-line
    M-r   Search and Replace
    M-w   copy-region

    C-<spacebar> Set mark at current position.

    ^X^C  Exit. Any unsaved files will require confirmation.
    ^X^F  Find file; read into a new buffer created from filename.
    ^X^S  Save current buffer to disk, using the filename associated with the buffer
    ^X^W  Write current buffer to disk. Type in a new filename at the prompt
    ^Xi   Insert file at point
    ^X=   Show Line Stats
    ^X^N  next-buffer
    ^Xn   next-buffer
    ^Xk   kill-buffer
    ^X1   delete-other-windows
    ^X2   split-window
    ^Xo   other-window

    Home  Beginning-of-line
    End   End-of-line
    Del   Delete character under cursor
    Left  Move left
    Right Move point right
    Up    Move to the previous line
    Down  Move to the next line
    Backspace delete caharacter on the left
