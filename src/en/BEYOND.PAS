{$R+,S+,I+,D-,T-,F-,V+,B-,N-,L+}
{$M 65520,130072,655360 }          { <=== very important ! ! ! }

(**************************************************************)
(*                    BEYOND.PAS                              *)
(*    loads programs into memory segments available beyond    *)
(*     video adapters. A max of 8 paramters can be passed.    *)
(*               (c) Ralf Preller 1.9.1988                    *)
(*                                                            *)
(* SYNTAX:                                                    *)
(*     BEYOND [path] progname.ext paraml param2 ...           *)
(*                                                            *)
(*            to be compiled with Turbo Pascal 4.0            *)
(**************************************************************)
PROGRAM Beyond;

USES
  Dos;

VAR
  Path,
  CmdLine : string;
  I       : byte;

BEGIN
Path := ParamStr(1) ;
CmdLine := ParamStr(2);
for I := 3 to 9 do CmdLine := CmdLine + ' ' + ParamStr(I);
while CmdLine [Length(CmdLine)] = ' ' do dec (CmdLine[0]) ;
Exec (Path, CmdLine);
if DosError <> 0 then begin
  writeln ('BEYOND *** DOS Error ', DosError, '. ',
    Path, ' not installed !!!');
  Halt(255) ;
  end
else Halt(DosExitCode) ;
END.
