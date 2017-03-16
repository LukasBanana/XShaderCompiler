
# Developer Notes #

Abstract Syntax Tree (AST)
--------------------------

Since there is no complete HLSL grammar specification I know of,
the overall structure of the AST comes from many trials and erros (and might still change).

Here is a brief overview how one of the most complex constructs, the **object identifiers**, are structured:
These object identifiers consist of several AST classes, but the primary class is called `VarIdent`,
which stands for "variable identifier", but it does also refer to functions, samplers, buffers etc.

***Note**: the following documented AST is still in developement, so it might not exactly reflect the current state!*

### `VarIdent` ###

`VarIdent` holds primarily an atomic **identifier string** and an optional **next variable identifier**.
It is traversed in **pre-order**, meaning the identifier string is used first, and then the sub nodes.
This AST node also has a list of **array indices** that can optionally appear between the sub nodes,
and a boolean member to specify wether the next sub node is a **static member**:

Specification:
```
VarIdent:
    String     identifier
    List<Expr> arrayIndices [Optional]
    Boolean    nextIsStatic [Optional]
    VarIdent   next         [Optional]
```

Example: `light.color.r`
```cs
VarIdent
 |-identifier = "light"
 `-next (VarIdent)
    `-identifier = "color"
       `-next (VarIdent)
          `-identifier = "r"
```

Example: `lights[3][6].position.x`
```cs
VarIdent
 |-identifier = "lights"
 |-arrayIndices[0] (LiteralExpr)
 |  `-literal = "3"
 |-arrayIndices[1] (LiteralExpr)
 |  `-literal = "6"
 `-next (VarIdent)
    `-identifier = "position"
       `-next (VarIdent)
          `-identifier = "x"
```

Example: `Light::globalAmbientColor.rgb`
```cs
VarIdent
 |-identifier = "Light"
 |-nextIsStatic = true
 `-next (VarIdent)
    `-identifier = "globalAmbientColor"
       `-next (VarIdent)
          `-identifier = "rgb"
```

This `VarIdent` is a standalone AST class, meaning it inherits directly from the base class `AST` and from `Expr` (for expression) for instance.

There are two more AST classes to represent array indices and variable names in expressions: `ArrayAccessExpr` and `SuffixExpr`.

### `ArrayAccessExpr` ###

`ArrayAccessExpr` only holds a **prefix expression** (of any expression type) and a list of **array indices**.
It is traversed in **post-order**, meaning the sub-expression is used, and then the array indices.

This is in the opposite order than `VarIdent` to support left-hand sub expressions of multiple types.
Otherwise the array indices must be contained in many other AST classes, like is the case for `VarIdent`.

Specification:
```
ArrayAccessExpr:
    Expr       prefixExpression
    List<Expr> arrayIndices
```

Example: `getLightList()[42]`
```cs
ArrayAccessExpr
 |-prefixExpression (FunctionCallExpr)
 |  `-call
 |     `-identifier = "getLightList"
 `-arrayIndices[0] (LiteralExpr)
    `-literal = "42"
```

Example: `( getLightList() )[42]`
```cs
ArrayAccessExpr
 |-prefixExpression (BracketExpr)
 |  `-subExpression (FunctionCallExpr)
 |     `-call (FunctionCall)
 |        `-identifier = "getLightList"
 `-arrayIndices[0] (LiteralExpr)
    `-literal = "42"
```

The second example shows, that using the post-order avoids,
that array indices must be contained (and also handled by all AST visitors!) in multiple AST classes,
here in `BracketExpr` and `FunctionCallExpr`.

### `VarAccessExpr` ###

`VarAccessExpr` holds an *optional* **prefix expression**, a **variable identifier**, and an *optional* **assignment expression**.
It is traversed in **in-order**, meaning the (left hand side) prefix expression is traversed first (if used),
then the variable identifier, and the assignment expression (if used).

Specification:
```
VarAccessExpr:
    Expr     prefixExpression     [Optional]
    VarIdent variableIdentifier
    Expr     assignmentExpression [Optional]
```

Example: `getLight().color`
```cs
VarAccessExpr
 |-prefixExpression (FunctionCallExpr)
 |  `-call
 |     `-identifier = "getLight"
 `-variableIdentifier (VarIdent)
    `-identifier = "color"
```

Example: `( getLight() ).color`
```cs
VarAccessExpr
 |-prefixExpression (BracketExpr)
 |  `-subExpression (FunctionCallExpr)
 |     `-call (FunctionCall)
 |        `-identifier = "getLight"
 `-variableIdentifier (VarIdent)
    `-identifier = "color"
```

Example: `light.shininess = 1`
```cs
VarAccessExpr
 |-variableIdentifier (VarIdent)
 |  `-identifier = "light"
 |     `-next (VarIdent)
 |        `-identifier = "shininess"
 `-assignmentExpression (LiteralExpr)
    `-literal = "1"
```

### `FunctionCallExpr` ###

`FunctionCallExpr` holds an *optional* **prefix expression** and a **function call**,
whereas this function call node holds again a variable identifier as a name of the function.
It is traversed in **pre-order**, meaning the (left hand side) prefix expression is traversed first (if used)
and then the function call.

Specification:
```
FunctionCallExpr:
    Expr         prefixExpression [Optional]
    FunctionCall call
```

Example: `scene.getLight().getColor()`
```cs
FunctionCallExpr
 |-prefixExpression (FunctionCallExpr)
 |  `-call (FunctionCall)
 |     `-functionName (VarIdent)
 |        `-identifier = "scene"
 |           `-next (VarIdent)
 |              `-identifier = "getLight"
 `-call (FunctionCall)
    `-functionName (VarIdent)
       `-identifier = "getColor"
```

### Summary ###

Now here is an example how this tree of identifiers can work in combination.
Consider the following structure declarations:
```cs
struct Material
{
    float getShininess();
};

struct Light
{
    Material material;
};

struct Scene
{
    static Scene getMain();
    Light[2] getLights();
};
```

Example: `( Scene::getMain().getLights() )[1].material.getShininess()`
```cs
FunctionCallExpr
 |-prefixExpression (ArrayAccessExpr)
 |  |-prefixExpression (BracketExpr)
 |  |  `-subExpression (FunctionCallExpr)
 |  |     |-prefixExpression (FunctionCallExpr)
 |  |     |  `-call (FunctionCall)
 |  |     |     `-functionName (VarIdnet)
 |  |     |        |-identifier = "Scene"
 |  |     |        |-nextIsStatic = true
 |  |     |        `-next (VarIdent)
 |  |     |           `-identifier = "getMain"
 |  |     `-call (FunctionCall)
 |  |        `-identifier = "getLightList"
 |  `-arrayIndices[0] (LiteralExpr)
 |     `-literal = "1"
 `-call (FunctionCall)
    `-functionName (VarIdent)
       `-identifier = "material"
          `-next (VarIdent)
             `-identifier = "getShininess"
```






