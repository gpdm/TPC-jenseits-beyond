{$R+,S+,I+,D-,T-,F-,V+,B-,N-,L+}
{$M 65520,130072,655360 }            { <=== ganz wichtig ! ! ! }

(**************************************************************)
(*                   JENSEITS.PAS                             *)
(*    laedt Programme in Speichererweiterung jenseits des     *)
(* Video-Adapters. Max.8 Parameter koennen uebergeben werden, *)
(*               (c) Ralf Preller 1.9.1988                    *)
(*                                                            *)
(* Aufruf:                                                    *)
(*     JENSEITS [pfad] progname.ext paraml param2 ...         *)
(*                                                            *)
(*            mit Turbo 4.0 zu kompilieren                    *)
(**************************************************************)
PROGRAM Jenseits;

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
  writeln ('JENSEITS *** DOS-Fehler ', DosError, '. ',
    Path, ' nicht installiert !!!');
  Halt(255) ;
  end
else Halt(DosExitCode) ;
END.
