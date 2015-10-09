# TODO

- All data is introspectable
- You can go up and down in the layers of abstraction

One of the goals is to have a language that isn't of handling "may", allows you
to safely handle "when".

## Primitives

The primitives abstract over the byte order (big or little endian) and which are
available depend on the processor you're targetting.

## Encodings

Allow the programmer to define their own encodings for primitives. These should
either be general, or tied to a specific primitive. By default the only
operations you can do on raw primitives is bit manipulation.

### Numbers

- [ ] Natural
- [ ] Integer
- [ ] Real

### Characters

- [ ] ASCII Characters
- [ ] UTF-8 Characters

## Linux Userspace API

- [ ] exit
- [ ] exit_group
- [ ] read
- [ ] write
